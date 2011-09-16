/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purchaseOrderItem.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

#include "taxDetail.h"
#include "itemCharacteristicDelegate.h"
#include "itemSourceSearch.h"
#include "vendorPriceList.h"
#include <QSqlError>

purchaseOrderItem::purchaseOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _vendorItemNumberList->setMaximumWidth(25);
#else
  _listPrices->setMinimumWidth(60);
#endif

  _invVendUOMRatio = 1;
  _minimumOrder = 0;
  _orderMultiple = 0;

  connect(_ordered, SIGNAL(lostFocus()), this, SLOT(sDeterminePrice()));
  connect(_inventoryItem, SIGNAL(toggled(bool)), this, SLOT(sInventoryItemToggled(bool)));
  connect(_item, SIGNAL(privateIdChanged(int)), this, SLOT(sFindWarehouseItemsites(int)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemInfo(int)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_vendorItemNumberList, SIGNAL(clicked()), this, SLOT(sVendorItemNumberList()));
  connect(_notesButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_listPrices, SIGNAL(clicked()), this, SLOT(sVendorListPrices()));
  connect(_taxLit, SIGNAL(leftClickedURL(QString)), this, SLOT(sTaxDetail()));  // new slot added for tax url //
  connect(_extendedPrice, SIGNAL(valueChanged()), this, SLOT(sCalculateTax()));  // new slot added for price //
  connect(_taxtype, SIGNAL(newID(int)), this, SLOT(sCalculateTax()));            // new slot added for taxtype //

  _parentwo = -1;
  _parentso = -1;
  _itemsrcid = -1;
  _taxzoneid = -1;   //  _taxzoneid  added // 
  _orderQtyCache = -1;

  _overriddenUnitPrice = false;

  _ordered->setValidator(omfgThis->qtyVal());

  _project->setType(ProjectLineEdit::PurchaseOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  _minOrderQty->setValidator(omfgThis->qtyVal());
  _orderQtyMult->setValidator(omfgThis->qtyVal());
  _received->setValidator(omfgThis->qtyVal());
  _invVendorUOMRatio->setValidator(omfgThis->ratioVal());

  q.exec("SELECT DISTINCT itemsrc_manuf_name FROM itemsrc ORDER BY 1;");
  for (int i = 0; q.next(); i++)
    _manufName->append(i, q.value("itemsrc_manuf_name").toString());
  _manufName->setId(-1);

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  //If not Revision Control, hide controls
  if (!_metrics->boolean("RevControl"))
   _tab->removeTab(_tab->indexOf(_revision));
   
  adjustSize();
  
  //TO DO: Implement later
  _taxRecoverable->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
purchaseOrderItem::~purchaseOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void purchaseOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse purchaseOrderItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  bool     haveQty  = FALSE;
  bool     haveDate = FALSE;



  param = pParams.value("warehous_id", &valid);
  if (valid)
    _preferredWarehouseid = param.toInt();

  param = pParams.value("parentWo", &valid);
  if (valid)
    _parentwo = param.toInt();

  param = pParams.value("parentSo", &valid);
  if (valid)
    _parentso = param.toInt();

  if (_metrics->boolean("CopyPRtoPOItem"))
  {
    param = pParams.value("pr_releasenote", &valid);
    if(valid)
      _notes->setText(param.toString());
  }

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _poheadid = param.toInt();

    q.prepare( "SELECT pohead_taxzone_id, pohead_number, pohead_orderdate, pohead_status, " // pohead_taxzone_id added
               "       vend_id, vend_restrictpurch, pohead_curr_id "
               "FROM pohead, vend "
               "WHERE ( (pohead_vend_id=vend_id)"
               " AND (pohead_id=:pohead_id) );" );
    q.bindValue(":pohead_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
      _poStatus = q.value("pohead_status").toString();
      _unitPrice->setEffective(q.value("pohead_orderdate").toDate());
      _unitPrice->setId(q.value("pohead_curr_id").toInt());
	  _taxzoneid=q.value("pohead_taxzone_id").toInt();   // added  to pick up tax zone id.
	  _tax->setEffective(q.value("pohead_orderdate").toDate());
      _tax->setId(q.value("pohead_curr_id").toInt());

      if (q.value("vend_restrictpurch").toBool())
      {
        _item->setQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                                  "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                                  "                uom_name, item_type, item_config, item_active "
                                  "FROM item, itemsite, itemsrc, uom  "
                                  "WHERE ( (itemsite_item_id=item_id)"
                                  " AND (itemsrc_item_id=item_id)"
                                  " AND (item_inv_uom_id=uom_id)"
                                  " AND (itemsite_active)"
                                  " AND (item_active)"
                                  " AND (itemsrc_active)"
                                  " AND (itemsrc_vend_id=%1) ) "
                                  "ORDER BY item_number" )
                         .arg(q.value("vend_id").toInt()) );
        _item->setValidationQuery( QString( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                                            "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                                            "                uom_name, item_type, item_config, item_active "
                                            "FROM item, itemsite, itemsrc, uom  "
                                            "WHERE ( (itemsite_item_id=item_id)"
                                            " AND (itemsrc_item_id=item_id)"
                                            " AND (item_inv_uom_id=uom_id)"
                                            " AND (itemsite_active)"
                                            " AND (item_active)"
                                            " AND (itemsrc_active)"
                                            " AND (itemsrc_vend_id=%1) "
                                            " AND (itemsite_item_id=:item_id) ) ")
                                   .arg(q.value("vend_id").toInt()) );
      }
      else
      {
        _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured |
		               ItemLineEdit::cTooling | ItemLineEdit::cActive);
        _item->setDefaultType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cActive);
      }
    }
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return UndefinedError;
    }
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    q.prepare( "SELECT pohead_number, pohead_id "
               "FROM pohead, poitem "
               "WHERE ( (pohead_id=poitem_pohead_id) "
               " AND (poitem_id=:poitem_id) );" );
    q.bindValue(":poitem_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
	  _poheadid = q.value("pohead_id").toInt();
    }

    populate();
	sCalculateTax();
  }
  // connect here and not in the .ui to avoid timing issues at initialization
  connect(_unitPrice, SIGNAL(valueChanged()), this, SLOT(sPopulateExtPrice()));

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('poitem_poitem_id_seq') AS poitem_id;");
      if (q.first())
        _poitemid = q.value("poitem_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      if(_parentso != -1)
      {
        q.prepare( "INSERT INTO charass"
                   "      (charass_target_type, charass_target_id,"
                   "       charass_char_id, charass_value) "
                   "SELECT 'PI', :orderid, charass_char_id, charass_value"
                   "  FROM charass"
                   " WHERE ((charass_target_type='SI')"
                   "   AND  (charass_target_id=:soitem_id));");
        q.bindValue(":orderid", _poitemid);
        q.bindValue(":soitem_id", _parentso);
        q.exec();
      }

      q.prepare( "SELECT (COALESCE(MAX(poitem_linenumber), 0) + 1) AS _linenumber "
                 "FROM poitem "
                 "WHERE (poitem_pohead_id=:pohead_id);" );
      q.bindValue(":pohead_id", _poheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("_linenumber").toString());
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

        return UndefinedError;
      }

      if (!_item->isValid())
        _item->setFocus();
      else if (!haveQty)
        _ordered->setFocus();
      else if (!haveDate)
        _dueDate->setFocus();

      _comments->setEnabled(FALSE);
      _tab->setTabEnabled(_tab->indexOf(_demandTab), FALSE);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _typeGroup->setEnabled(FALSE);

      _ordered->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _typeGroup->setEnabled(FALSE);
      _vendorItemNumber->setEnabled(FALSE);
      _vendorItemNumberList->setEnabled(FALSE);
      _vendorDescrip->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _ordered->setEnabled(FALSE);
      _unitPrice->setEnabled(FALSE);
      _freight->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _comments->setReadOnly(TRUE);
      _project->setEnabled(FALSE);
      _taxtype->setEnabled(FALSE);
      _taxRecoverable->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
  }
  
  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    sPopulateItemSourceInfo(param.toInt());

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _ordered->setDouble((param.toDouble()/_invVendUOMRatio));

    if (_item->isValid())
      sDeterminePrice();

    haveQty = TRUE;
  }

  param = pParams.value("dueDate", &valid);
  if (valid)
  {
    _dueDate->setDate(param.toDate());
    haveDate = TRUE;
  }

  param = pParams.value("prj_id", &valid);
  if (valid)
    _project->setId(param.toInt());

  if(_parentso != -1)
  {
    q.prepare("SELECT coitem_prcost"
              "  FROM coitem"
              " WHERE (coitem_id=:parentso); ");
    q.bindValue(":parentso", _parentso);
    q.exec();
    if(q.first())
    {
      if(q.value("coitem_prcost").toDouble() > 0)
      {
        _overriddenUnitPrice = true;
        _unitPrice->setLocalValue(q.value("coitem_prcost").toDouble());
        sPopulateExtPrice();
      }
    }
  }

  return NoError;
}

void purchaseOrderItem::populate()
{
  q.prepare( "SELECT pohead_number, poitem_linenumber, pohead_taxzone_id, "
             "       COALESCE(poitem_itemsite_id,-1) AS poitem_itemsite_id,"
             "       COALESCE(poitem_itemsrc_id,-1) AS poitem_itemsrc_id, "
             "       poitem_vend_item_number, poitem_vend_item_descrip,"
             "       poitem_vend_uom,"
             "       poitem_invvenduomratio,"
             "       COALESCE(poitem_expcat_id,-1) AS poitem_expcat_id, "
	     "       COALESCE(pohead_cohead_id, -1) AS pohead_cohead_id, "
	     "       COALESCE(poitem_wohead_id, -1) AS poitem_wohead_id, "
             "       CASE WHEN (COALESCE(pohead_cohead_id, -1) != -1) THEN :sonum "
             "            WHEN (COALESCE(poitem_wohead_id, -1) != -1) THEN :wonum "
             "            ELSE '' "
             "       END AS demand_type, "
             "       CASE WHEN (COALESCE(pohead_cohead_id, -1) != -1) THEN cohead_number "
             "            WHEN (COALESCE(poitem_wohead_id, -1) != -1) THEN CAST(wo_number AS text) "
             "         ELSE '' "
             "       END AS order_number, "
             "       CASE WHEN pohead_cohead_id IS NOT NULL THEN "
             "              CAST(coitem_linenumber AS text) "
             "            WHEN poitem_wohead_id IS NOT NULL THEN "
             "              CAST(wo_subnumber AS text) "
             "            ELSE '' "
             "       END AS orderline_number, "
             "       poitem_duedate,"
             "       poitem_qty_ordered,"
             "       poitem_qty_received,"
             "       pohead_curr_id, pohead_orderdate, "
             "       poitem_unitprice,"
             "       poitem_freight,"
             "       poitem_unitprice * poitem_qty_ordered AS f_extended,"
             "       poitem_taxtype_id, poitem_tax_recoverable, "
             "       poitem_comments, poitem_prj_id,"
             "       poitem_bom_rev_id,poitem_boo_rev_id, "
             "       poitem_manuf_name, poitem_manuf_item_number, "
             "       poitem_manuf_item_descrip, "
             "       COALESCE(coitem_prcost, 0.0) AS overrideCost "
             "  FROM pohead JOIN poitem ON (poitem_pohead_id=pohead_id)"
             "  LEFT OUTER JOIN coitem  ON (poitem_soitem_id=coitem_id)"
             "  LEFT OUTER JOIN cohead  ON (cohead_id = coitem_cohead_id)"
             "  LEFT OUTER JOIN wo      ON (poitem_wohead_id = wo_id) "
             "WHERE (poitem_id=:poitem_id);" );
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":sonum",     tr("Sales Order #")),
  q.bindValue(":wonum",     tr("Work Order #")),
  q.exec();
  if (q.first())
  {
    _poNumber->setText(q.value("pohead_number").toString());
    _lineNumber->setText(q.value("poitem_linenumber").toString());
    _taxzoneid=q.value("pohead_taxzone_id").toInt();   // added  to pick up tax zone id.
    _dueDate->setDate(q.value("poitem_duedate").toDate());
    _ordered->setDouble(q.value("poitem_qty_ordered").toDouble());
    _orderQtyCache = _ordered->toDouble();
    _received->setDouble(q.value("poitem_qty_received").toDouble());
    _unitPrice->set(q.value("poitem_unitprice").toDouble(),
		    q.value("pohead_curr_id").toInt(),
		    q.value("pohead_orderdate").toDate(), false);
    _freight->setLocalValue(q.value("poitem_freight").toDouble());
    _extendedPrice->setLocalValue(q.value("f_extended").toDouble());
    _taxtype->setId(q.value("poitem_taxtype_id").toInt());
    _taxRecoverable->setChecked(q.value("poitem_tax_recoverable").toBool());
    _notes->setText(q.value("poitem_comments").toString());
    _project->setId(q.value("poitem_prj_id").toInt());
    if(q.value("overrideCost").toDouble() > 0)
      _overriddenUnitPrice = true;

    if(q.value("pohead_cohead_id") != -1)
    {
      _ordered->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
	  _soLit->setText(q.value("demand_type").toString());
	  _so->setText(q.value("order_number").toString());
	  _soLine->setText(q.value("orderline_number").toString());
    }
    else if(q.value("poitem_wohead_id") != -1)
    {
      _soLit->setText(q.value("demand_type").toString());
	  _so->setText(q.value("order_number").toString());
	  _soLine->setText(q.value("orderline_number").toString());
	}
	else
	  _tab->setTabEnabled(_tab->indexOf(_demandTab), FALSE);

    if (q.value("poitem_itemsite_id").toInt() == -1)
    {
      _nonInventoryItem->setChecked(TRUE);
      _expcat->setId(q.value("poitem_expcat_id").toInt());
      sPopulateItemSourceInfo(-1);

      _vendorItemNumber->setText(q.value("poitem_vend_item_number").toString());
      _vendorDescrip->setText(q.value("poitem_vend_item_descrip").toString());
      _vendorUOM->setText(q.value("poitem_vend_uom").toString());
      _uom->setText(q.value("poitem_vend_uom").toString());
    }
    else
    {
      _inventoryItem->setChecked(TRUE);
      _item->setItemsiteid(q.value("poitem_itemsite_id").toInt());
      sPopulateItemSourceInfo(_item->id());
      if (_metrics->boolean("RevControl"))
      {
        _bomRevision->setId(q.value("poitem_bom_rev_id").toInt());
        _booRevision->setId(q.value("poitem_boo_rev_id").toInt());
      }
    }

    _itemsrcid = q.value("poitem_itemsrc_id").toInt();
    _vendorItemNumber->setText(q.value("poitem_vend_item_number").toString());
    _vendorDescrip->setText(q.value("poitem_vend_item_descrip").toString());
    
    if (_itemsrcid == -1)
    {
      _vendorUOM->setText(q.value("poitem_vend_uom").toString());
      _uom->setText(q.value("poitem_vend_uom").toString());
      _invVendorUOMRatio->setDouble(q.value("poitem_invvenduomratio").toDouble());
      _invVendUOMRatio = q.value("poitem_invvenduomratio").toDouble();
      _manufName->setText(q.value("poitem_manuf_name").toString());
      if (_manufName->id() < 0)
      {
        _manufName->append(_manufName->count(),
                           q.value("poitem_manuf_name").toString());
        _manufName->setText(q.value("poitem_manuf_name").toString());
      }
      _manufItemNumber->setText(q.value("poitem_manuf_item_number").toString());
      _manufItemDescrip->setText(q.value("poitem_manuf_item_descrip").toString());
    }
    else
    {
      q.prepare( "SELECT itemsrc_id, itemsrc_vend_item_number,"
                 "       itemsrc_vend_item_descrip, itemsrc_vend_uom,"
                 "       itemsrc_minordqty,"
                 "       itemsrc_multordqty,"
                 "       itemsrc_invvendoruomratio, "
                 "       itemsrc_manuf_name, "
                 "       itemsrc_manuf_item_number, "
                 "       itemsrc_manuf_item_descrip "
                 "FROM itemsrc "
                 "WHERE (itemsrc_id=:itemsrc_id);" );
      q.bindValue(":itemsrc_id", _itemsrcid);
      q.exec();
      if (q.first())
      {
//        _vendorItemNumber->setEnabled(FALSE);
//        _vendorItemNumberList->setEnabled(FALSE);
//        _vendorDescrip->setEnabled(FALSE);
        _vendorUOM->setEnabled(FALSE);
        _manufName->setEnabled(FALSE);
        _manufItemNumber->setEnabled(FALSE);
        _manufItemDescrip->setEnabled(FALSE);

        if(_vendorItemNumber->text().isEmpty())
          _vendorItemNumber->setText(q.value("itemsrc_vend_item_number").toString());
        if(_vendorDescrip->toPlainText().isEmpty())
          _vendorDescrip->setText(q.value("itemsrc_vend_item_descrip").toString());
        _vendorUOM->setText(q.value("itemsrc_vend_uom").toString());
        _uom->setText(q.value("itemsrc_vend_uom").toString());
        _minOrderQty->setDouble(q.value("itemsrc_minordqty").toDouble());
        _orderQtyMult->setDouble(q.value("itemsrc_multordqty").toDouble());
        _invVendorUOMRatio->setDouble(q.value("itemsrc_invvendoruomratio").toDouble());

        _invVendUOMRatio = q.value("itemsrc_invvendoruomratio").toDouble();
        _minimumOrder = q.value("itemsrc_minordqty").toDouble();
        _orderMultiple = q.value("itemsrc_multordqty").toDouble();
        
        if(_manufName->currentText().isEmpty())
          _manufName->setText(q.value("itemsrc_manuf_name").toString());
        if(_manufItemNumber->text().isEmpty())
          _manufItemNumber->setText(q.value("itemsrc_manuf_item_number").toString());
        if(_manufItemDescrip->toPlainText().isEmpty())
          _manufItemDescrip->setText(q.value("itemsrc_manuf_item_descrip").toString());
      }
//  ToDo
    }

    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='PI'"
               "      AND b.charass_target_id=:poitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, _item->id(), Qt::UserRole);
      row++;
    }

    _comments->setId(_poitemid);
  }
}

void purchaseOrderItem::sSave()
{
  if (!_inventoryItem->isChecked() && _expcat->id() == -1)
  {
    QMessageBox::critical( this, tr("Expense Category Required"),
                           tr("<p>You must specify an Expense Category for this non-Inventory Item before you may save it.") );
    return;
  }

  if (_inventoryItem->isChecked() && !_item->isValid())
  {
    QMessageBox::critical( this, tr("No Item Selected"),
                           tr("<p>You must select an Item Number before you may save.") );
    return;
  }

  if (_inventoryItem->isChecked() && _warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("No Site Selected"),
                           tr("<p>You must select a Supplying Site before you may save.") );
    return;
  }

  if (_ordered->toDouble() == 0.0)
  {
    if (QMessageBox::critical( this, tr("Zero Order Quantity"),
                               tr( "<p>The quantity that you are ordering is zero. "
                                   "<p>Do you wish to Continue or Change the Order Qty?" ),
                               QString("&Continue"), QString("Change Order &Qty."), QString::null, 1, 1 ) == 1)
    {
      _ordered->setFocus();
      return;
    }
  }

  if (_ordered->toDouble() < _minimumOrder)
  {
    if (QMessageBox::critical( this, tr("Invalid Order Quantity"),
                               tr( "<p>The quantity that you are ordering is below the Minimum Order Quantity for this "
                                   "Item Source.  You may continue but this Vendor may not honor pricing or delivery quotations. "
                                   "<p>Do you wish to Continue or Change the Order Qty?" ),
                               QString("&Continue"), QString("Change Order &Qty."), QString::null, 1, 1 ) == 1)
    {
      _ordered->setFocus();
      return;
    }
  }

  if ((int)_orderMultiple)
  {
    if (qRound(_ordered->toDouble()) % (int)_orderMultiple)
    {
      if (QMessageBox::critical( this, tr("Invalid Order Quantity"),
                                 tr( "<p>The quantity that you are ordering does not fall within the Order Multiple for this "
                                     "Item Source.  You may continue but this Vendor may not honor pricing or delivery quotations. "
                                     "<p>Do you wish to Continue or Change the Order Qty?" ),
                                 QString("&Continue"), QString("Change Order &Qty."), QString::null, 1, 1 ) == 1)
      {
        _ordered->setFocus();
        return;
      }
    }
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::critical( this, tr("Invalid Due Date"),
                           tr("<p>You must enter a due date before you may save this Purchase Order Item.") );
    _dueDate->setFocus();
    return;
  }

  if (_dueDate->date() < _earliestDate->date())
  {
    if (QMessageBox::critical( this, tr("Invalid Due Date "),
                               tr( "<p>The Due Date that you are requesting does not fall within the Lead Time Days for this "
                                   "Item Source.  You may continue but this Vendor may not honor pricing or delivery quotations "
                                   "or may not be able to deliver by the requested Due Date. "
                                   "<p>Do you wish to Continue or Change the Due Date?" ),
                               QString("&Continue"), QString("Change Order &Due Date"), QString::null, 1, 1 ) == 1)
    {
      _dueDate->setFocus();
      return;
    }
  }

  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO poitem "
               "( poitem_id, poitem_pohead_id, poitem_status, poitem_linenumber,"
               "  poitem_taxtype_id, poitem_tax_recoverable,"
               "  poitem_itemsite_id, poitem_expcat_id,"
               "  poitem_itemsrc_id, poitem_vend_item_number, poitem_vend_item_descrip,"
               "  poitem_vend_uom, poitem_invvenduomratio,"
               "  poitem_qty_ordered,"
               "  poitem_unitprice, poitem_freight, poitem_duedate, "
               "  poitem_bom_rev_id, poitem_boo_rev_id, "
               "  poitem_comments, poitem_prj_id, poitem_stdcost, poitem_manuf_name, "
               "  poitem_manuf_item_number, poitem_manuf_item_descrip, poitem_rlsd_duedate ) "
               "VALUES "
               "( :poitem_id, :poitem_pohead_id, :status, :poitem_linenumber,"
               "  :poitem_taxtype_id, :poitem_tax_recoverable,"
               "  :poitem_itemsite_id, :poitem_expcat_id,"
               "  :poitem_itemsrc_id, :poitem_vend_item_number, :poitem_vend_item_descrip,"
               "  :poitem_vend_uom, :poitem_invvenduomratio,"
               "  :poitem_qty_ordered,"
               "  :poitem_unitprice, :poitem_freight, :poitem_duedate, "
               "  :poitem_bom_rev_id, :poitem_boo_rev_id, "
               "  :poitem_comments, :poitem_prj_id, stdcost(:item_id), :poitem_manuf_name, "
               "  :poitem_manuf_item_number, :poitem_manuf_item_descrip, :poitem_duedate) ;" );

    q.bindValue(":status", _poStatus);
    q.bindValue(":item_id", _item->id());

    if (_inventoryItem->isChecked())
    {
      XSqlQuery itemsiteid;
      itemsiteid.prepare( "SELECT itemsite_id "
                          "FROM itemsite "
                          "WHERE ( (itemsite_item_id=:item_id)"
                          " AND (itemsite_warehous_id=:warehous_id) );" );
      itemsiteid.bindValue(":item_id", _item->id());
      itemsiteid.bindValue(":warehous_id", _warehouse->id());
      itemsiteid.exec();
      if (itemsiteid.first())
        q.bindValue(":poitem_itemsite_id", itemsiteid.value("itemsite_id").toInt());
      else
      {
        QMessageBox::critical( this, tr("Invalid Item/Site"),
                               tr("<p>The Item and Site you have selected does not appear to be a valid combination. "
                                  "Make sure you have a Site selected and that there is a valid itemsite for "
                                  "this Item and Site combination.") );
        return;
      }
    }
    else
    {
      q.bindValue(":poitem_expcat_id", _expcat->id());
    }
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE poitem "
               "SET poitem_itemsrc_id=:poitem_itemsrc_id,"   
               "    poitem_taxtype_id=:poitem_taxtype_id,"
               "    poitem_tax_recoverable=:poitem_tax_recoverable,"
               "    poitem_vend_item_number=:poitem_vend_item_number,"
               "    poitem_vend_item_descrip=:poitem_vend_item_descrip,"
               "    poitem_vend_uom=:poitem_vend_uom, poitem_invvenduomratio=:poitem_invvenduomratio,"
               "    poitem_qty_ordered=:poitem_qty_ordered, poitem_unitprice=:poitem_unitprice,"
               "    poitem_freight=:poitem_freight,"
               "    poitem_duedate=:poitem_duedate, poitem_comments=:poitem_comments,"
               "    poitem_prj_id=:poitem_prj_id, "
               "    poitem_bom_rev_id=:poitem_bom_rev_id, "
               "    poitem_boo_rev_id=:poitem_boo_rev_id, "
               "    poitem_manuf_name=:poitem_manuf_name, "
               "    poitem_manuf_item_number=:poitem_manuf_item_number, "
               "    poitem_manuf_item_descrip=:poitem_manuf_item_descrip "
               "WHERE (poitem_id=:poitem_id);" );

  q.bindValue(":poitem_id", _poitemid);
  if (_taxtype->id() != -1)
    q.bindValue(":poitem_taxtype_id", _taxtype->id());
  q.bindValue(":poitem_tax_recoverable", QVariant(_taxRecoverable->isChecked()));
  q.bindValue(":poitem_pohead_id", _poheadid);
  q.bindValue(":poitem_linenumber", _lineNumber->text().toInt());
  if (_itemsrcid != -1)
    q.bindValue(":poitem_itemsrc_id", _itemsrcid);
  q.bindValue(":poitem_vend_item_number", _vendorItemNumber->text());
  q.bindValue(":poitem_vend_item_descrip", _vendorDescrip->toPlainText());
  q.bindValue(":poitem_vend_uom", _vendorUOM->text());
  q.bindValue(":poitem_invvenduomratio", _invVendorUOMRatio->toDouble());
  q.bindValue(":poitem_qty_ordered", _ordered->toDouble());
  q.bindValue(":poitem_unitprice", _unitPrice->localValue());
  q.bindValue(":poitem_freight", _freight->localValue());
  q.bindValue(":poitem_duedate", _dueDate->date());
  q.bindValue(":poitem_manuf_name", _manufName->currentText());
  q.bindValue(":poitem_manuf_item_number", _manufItemNumber->text());
  q.bindValue(":poitem_manuf_item_descrip", _manufItemDescrip->toPlainText());
  q.bindValue(":poitem_comments", _notes->toPlainText());
  if (_project->isValid())
    q.bindValue(":poitem_prj_id", _project->id());
  if (_metrics->boolean("RevControl"))
  {
    q.bindValue(":poitem_bom_rev_id", _bomRevision->id());
    q.bindValue(":poitem_boo_rev_id", _booRevision->id());
  }
  q.exec();

  if (_parentwo != -1)
  {
    q.prepare("UPDATE poitem SET poitem_wohead_id=:parentwo WHERE (poitem_id=:poitem_id);");
    q.bindValue(":parentwo", _parentwo);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
  }

  if (_parentso != -1)
  {
    q.prepare("UPDATE poitem SET poitem_soitem_id=:parentso WHERE (poitem_id=:poitem_id);");
    q.bindValue(":parentso", _parentso);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
  }

  if ( _mode != cView )
  {
    q.prepare("SELECT updateCharAssignment('PI', :target_id, :char_id, :char_value);");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_id", _poitemid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
    }
  }

  done(_poitemid);
}

void purchaseOrderItem::sPopulateExtPrice()
{
  _extendedPrice->setLocalValue(_ordered->toDouble() * _unitPrice->localValue());
}

void purchaseOrderItem::sFindWarehouseItemsites( int id )
{
  _warehouse->findItemsites(id);
  if(_preferredWarehouseid > 0)
    _warehouse->setId(_preferredWarehouseid);
}

void purchaseOrderItem::sPopulateItemInfo(int pItemid)
{
  XSqlQuery item;
  
  if (pItemid != -1 && _mode == cNew)
  {
    // Reset order qty cache
    _orderQtyCache = -1;
    
    if(_metrics->boolean("RequireStdCostForPOItem"))
    {
      item.prepare("SELECT stdCost(:item_id) AS result");
      item.bindValue(":item_id", pItemid);
      item.exec();
      if(item.first() && item.value("result").toDouble() == 0.0)
      {
        QMessageBox::critical( this, tr("Selected Item Missing Cost"),
                tr("<p>The selected item has no Std. Costing information. "
                   "Please see your controller to correct this situation "
                   "before continuing."));
        _item->setId(-1);
        return;
      }
    }
    
    item.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='PI'"
               "      AND b.charass_target_id=:poitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    item.bindValue(":item_id", pItemid);
    item.bindValue(":poitem_id", _poitemid);
    item.exec();
    int row = 0;
    _itemchar->removeRows(0, _itemchar->rowCount());
    QModelIndex idx;
    while(item.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, item.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, item.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, item.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }
    
    item.prepare("SELECT itemsrc_id " 
                 "FROM itemsrc, pohead "
                 "WHERE ( (itemsrc_vend_id=pohead_vend_id)"
                 " AND (itemsrc_item_id=:item_id)"
                 " AND (pohead_id=:pohead_id) );" );
    item.bindValue(":item_id", pItemid);
    item.bindValue(":pohead_id", _poheadid);
    item.exec();
    if (item.size() == 1)
    {
      item.first();
	 
      if (item.value("itemsrc_id").toInt() != _itemsrcid)
        sPopulateItemSourceInfo(item.value("itemsrc_id").toInt());
    }
    else if (item.size() > 1)
    {
      bool isCurrent = false;
      while (item.next())
      {
        if (item.value("itemsrc_id").toInt() == _itemsrcid)
          isCurrent = true;
      }
      if (!isCurrent)
      {
        _vendorItemNumber->clear();
        sVendorItemNumberList();
      }
    }
    else
    {
      _itemsrcid = -1;
  
      _vendorItemNumber->clear();
      _vendorDescrip->clear();
      _vendorUOM->setText(_item->uom());
      _uom->setText(_item->uom());
      _minOrderQty->clear();
      _orderQtyMult->clear();
      _invVendorUOMRatio->setDouble(1.0);
      _earliestDate->setDate(omfgThis->dbDate());
      _manufName->setId(-1);
      _manufItemNumber->clear();
      _manufItemDescrip->clear();
  
      _invVendUOMRatio = 1;
      _minimumOrder = 0;
      _orderMultiple = 0;
    }


	item.prepare("SELECT getItemTaxType(:item_id, pohead_taxzone_id) AS taxtype_id "
	             "FROM pohead WHERE (pohead_id=:pohead_id);");
  item.bindValue(":item_id", pItemid);
	item.bindValue(":pohead_id", _poheadid);
  item.exec();
	if(item.first())
    _taxtype->setId(item.value("taxtype_id").toInt());

	item.prepare("SELECT item_tax_recoverable "
	             "FROM item WHERE (item_id=:item_id);");
  item.bindValue(":item_id", pItemid);
  item.exec();
	if(item.first())
    _taxRecoverable->setChecked(item.value("item_tax_recoverable").toBool());

  }
}

void purchaseOrderItem::sPopulateItemSourceInfo(int pItemsrcid)
{
  XSqlQuery src;
  bool skipClear = false;
  if (!_item->isValid())
    skipClear = true;
  if (_mode == cNew)
  {
    if (pItemsrcid != -1)
    {
      src.prepare( "SELECT itemsrc_id, itemsrc_item_id, itemsrc_vend_item_number,"
                 "       itemsrc_vend_item_descrip, itemsrc_vend_uom,"
                 "       itemsrc_minordqty,"
                 "       itemsrc_multordqty,"
                 "       itemsrc_invvendoruomratio,"
                 "       (CURRENT_DATE + itemsrc_leadtime) AS earliestdate, "
                 "       itemsrc_manuf_name, "
                 "       itemsrc_manuf_item_number, "
                 "       itemsrc_manuf_item_descrip "
                 "FROM itemsrc "
                 "WHERE (itemsrc_id=:itemsrc_id);" );
      src.bindValue(":itemsrc_id", pItemsrcid);
      src.exec();
      if (src.first())
      {
        _itemsrcid = src.value("itemsrc_id").toInt();
        _item->setId(src.value("itemsrc_item_id").toInt());
  
        _vendorItemNumber->setText(src.value("itemsrc_vend_item_number").toString());
        _vendorDescrip->setText(src.value("itemsrc_vend_item_descrip").toString());
        _vendorUOM->setText(src.value("itemsrc_vend_uom").toString());
        _uom->setText(src.value("itemsrc_vend_uom").toString());
        _minOrderQty->setDouble(src.value("itemsrc_minordqty").toDouble());
        _orderQtyMult->setDouble(src.value("itemsrc_multordqty").toDouble());
        _invVendorUOMRatio->setDouble(src.value("itemsrc_invvendoruomratio").toDouble());
        _earliestDate->setDate(src.value("earliestdate").toDate());

        _invVendUOMRatio = src.value("itemsrc_invvendoruomratio").toDouble();
        _minimumOrder = src.value("itemsrc_minordqty").toDouble();
        _orderMultiple = src.value("itemsrc_multordqty").toDouble();
        
        _manufName->setCode(src.value("itemsrc_manuf_name").toString());
        _manufItemNumber->setText(src.value("itemsrc_manuf_item_number").toString());
        _manufItemDescrip->setText(src.value("itemsrc_manuf_item_descrip").toString());

        if (_ordered->toDouble() != 0)
          sDeterminePrice();

        _ordered->setFocus();

        if(_metrics->boolean("UseEarliestAvailDateOnPOItem"))
          _dueDate->setDate(_earliestDate->date());

        skipClear = true;
      }
    }

    if(!skipClear)
    {
      _itemsrcid = -1;
  
      _vendorItemNumber->clear();
      _vendorDescrip->clear();
      _vendorUOM->setText(_item->uom());
      _uom->setText(_item->uom());
      _minOrderQty->clear();
      _orderQtyMult->clear();
      _invVendorUOMRatio->setDouble(1.0);
      _earliestDate->setDate(omfgThis->dbDate());
      _manufName->setId(-1);
      _manufItemNumber->clear();
      _manufItemDescrip->clear();
  
      _invVendUOMRatio = 1;
      _minimumOrder = 0;
      _orderMultiple = 0;
    }
  }
}

void purchaseOrderItem::sDeterminePrice()
{
  if ((_orderQtyCache != -1) &&
      (_orderQtyCache != _ordered->toDouble()))
  {
    if (QMessageBox::question(this, tr("Update Price?"),
                tr("<p>The Item qty. has changed. Do you want to update the Price?"),
                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
    {
      _orderQtyCache = _ordered->toDouble();
	  sPopulateExtPrice();
      return;
    }
  }
   
  if ( (!_overriddenUnitPrice) && 
      (_itemsrcid != -1) && 
      (_ordered->toDouble() != 0.0) &&
      (_orderQtyCache != _ordered->toDouble()))
  {
    q.prepare( "SELECT currToCurr(itemsrcp_curr_id, :curr_id, itemsrcp_price, :effective) "
       	       "AS new_itemsrcp_price "
               "FROM itemsrcp "
               "WHERE ( (itemsrcp_itemsrc_id=:itemsrc_id)"
               " AND (itemsrcp_qtybreak <= :qty) ) "
               "ORDER BY itemsrcp_qtybreak DESC "
               "LIMIT 1;" );
    q.bindValue(":itemsrc_id", _itemsrcid);
    q.bindValue(":qty", _ordered->toDouble());
    q.bindValue(":curr_id", _unitPrice->id());
    q.bindValue(":effective", _unitPrice->effective());
    q.exec();
    if (q.first())
      _unitPrice->setLocalValue(q.value("new_itemsrcp_price").toDouble());
    else
      _unitPrice->clear();
  }
  if (_ordered->toDouble() != 0.0)
    _orderQtyCache = _ordered->toDouble();
  sPopulateExtPrice();
}

void purchaseOrderItem::sInventoryItemToggled( bool yes )
{
  if(yes)
    sPopulateItemSourceInfo(_item->id());
  else
    sPopulateItemSourceInfo(-1);
}

void purchaseOrderItem::sVendorItemNumberList()
{
  ParameterList params;

  q.prepare( "SELECT vend_id"
             "  FROM pohead, vend "
             " WHERE((pohead_vend_id=vend_id)"
             "   AND (pohead_id=:pohead_id));" );
  q.bindValue(":pohead_id", _poheadid);
  q.exec();
  if (q.first())
    params.append("vend_id", q.value("vend_id").toInt());
  if (!_vendorItemNumber->text().isEmpty())
    params.append("search", _vendorItemNumber->text());
  else if (_item->id() != -1)
    params.append("search", _item->itemNumber());
  itemSourceSearch newdlg(this, "", true);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
  {
    int itemsrcid = newdlg.itemsrcId();
    if(itemsrcid != -1)
    {
      _inventoryItem->setChecked(TRUE);
      sPopulateItemSourceInfo(itemsrcid);
    }
    else
    {
      _nonInventoryItem->setChecked(TRUE);
      _expcat->setId(newdlg.expcatId());
      _vendorItemNumber->setText(newdlg.vendItemNumber());
      _vendorDescrip->setText(newdlg.vendItemDescrip());
      _manufName->setText(newdlg.manufName());
      _manufItemNumber->setText(newdlg.manufItemNumber());
      _manufItemDescrip->setText(newdlg.manufItemDescrip());
    }
  }
}

void purchaseOrderItem::sHandleButtons()
{
  if (_notesButton->isChecked())
    _remarksStack->setCurrentIndex(0);
  else
    _remarksStack->setCurrentIndex(1);
}

void purchaseOrderItem::sVendorListPrices()
{
  ParameterList params;
  params.append("itemsrc_id", _itemsrcid);
  params.append("qty", _ordered->toDouble());
  vendorPriceList newdlg(this, "", TRUE);
  newdlg.set(params);
  if ( (newdlg.exec() == XDialog::Accepted))
  {
    if (_ordered->text().toDouble() < newdlg._selectedQty)
    {
      if (QMessageBox::question(this, tr("Update Quantity?"),
                  tr("<p>You must order at least %1 to qualify for this price. Do you want to update the Quantity?").arg(QString().setNum(newdlg._selectedQty)),
                  QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
        return;
  
      _ordered->setDouble(newdlg._selectedQty);
      _orderQtyCache = -1;
      sDeterminePrice();
    }
  }
}


void purchaseOrderItem::sCalculateTax()
{
  XSqlQuery calcq;

  calcq.prepare("SELECT calculateTax(pohead_taxzone_id,:taxtype_id,pohead_orderdate,pohead_curr_id,ROUND(:ext,2)) AS tax "
                "FROM pohead "
                "WHERE (pohead_id=:pohead_id); " );

  calcq.bindValue(":pohead_id", _poheadid);
  calcq.bindValue(":taxtype_id", _taxtype->id());
  calcq.bindValue(":ext", _extendedPrice->localValue());

  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());

  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
}

void purchaseOrderItem::sTaxDetail()    // new function added from raitem
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id",   _taxzoneid);
  params.append("taxtype_id",  _taxtype->id());
  params.append("date", _tax->effective());
  params.append("curr_id", _tax->id());
  params.append("subtotal", _extendedPrice->localValue());
  //params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxtype->id() != newdlg.taxtype())
      _taxtype->setId(newdlg.taxtype());
  }
}
