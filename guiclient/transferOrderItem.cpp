/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transferOrderItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "itemCharacteristicDelegate.h"
#include "itemSite.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"

transferOrderItem::transferOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_cancel,	SIGNAL(clicked()),      this, SLOT(sCancel()));
  connect(_freight,     SIGNAL(valueChanged()), this, SLOT(sCalculateTax()));
  connect(_freight,	SIGNAL(valueChanged()), this, SLOT(sChanged()));
  connect(_item,	SIGNAL(newId(int)),     this, SLOT(sChanged()));
  connect(_item,	SIGNAL(newId(int)),     this, SLOT(sPopulateItemInfo(int)));
  connect(_next,	SIGNAL(clicked()),      this, SLOT(sNext()));
  connect(_notes,	SIGNAL(textChanged()),  this, SLOT(sChanged()));
  connect(_prev,	SIGNAL(clicked()), this, SLOT(sPrev()));
  connect(_promisedDate,SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_qtyOrdered,  SIGNAL(lostFocus()), this, SLOT(sDetermineAvailability()));
  connect(_qtyOrdered,  SIGNAL(textChanged(const QString&)), this, SLOT(sChanged()));
  connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sDetermineAvailability()));
  connect(_showAvailability, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_showAvailability, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_showIndented,SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_taxLit, SIGNAL(leftClickedURL(QString)), this, SLOT(sTaxDetail()));
  connect(_warehouse,	SIGNAL(newID(int)), this, SLOT(sChanged()));
  connect(_warehouse,	SIGNAL(newID(int)), this, SLOT(sDetermineAvailability()));
  connect(_inventoryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButton()));

  _modified	= false;
  _canceling	= false;
  _error	= false;
  _originalQtyOrd	= 0.0;
  _saved = false;

  _availabilityLastItemid	= -1;
  _availabilityLastWarehousid	= -1;
  _availabilityLastSchedDate	= QDate();
  _availabilityLastShow		= false;
  _availabilityLastShowIndent	= false;
  _availabilityQtyOrdered	= 0.0;

  _item->setType(ItemLineEdit::cActive | (ItemLineEdit::cAllItemTypes_Mask ^ ItemLineEdit::cKit));
  _item->addExtraClause( QString("(itemsite_active)") ); // ItemLineEdit::cActive doesn't compare against the itemsite record

  _availability->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter,true, "bomitem_seqnumber");
  _availability->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _availability->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "item_descrip");
  _availability->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "uom_name");
  _availability->addColumn(tr("Pend. Alloc."),_qtyColumn, Qt::AlignRight, true, "pendalloc");
  _availability->addColumn(tr("Total Alloc."),_qtyColumn, Qt::AlignRight, true, "totalalloc");
  _availability->addColumn(tr("On Order"),    _qtyColumn, Qt::AlignRight, true, "ordered");
  _availability->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "qoh");
  _availability->addColumn(tr("Availability"),_qtyColumn, Qt::AlignRight, true, "totalavail");
  
  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  _qtyOrdered->setValidator(omfgThis->qtyVal());
  _shippedToDate->setPrecision(omfgThis->qtyVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unallocated->setPrecision(omfgThis->qtyVal());
  _onOrder->setPrecision(omfgThis->qtyVal());
  _available->setPrecision(omfgThis->qtyVal());

  if (!_metrics->boolean("UsePromiseDate"))
  {
    _promisedDateLit->hide();
    _promisedDate->hide();
  }

  _comments->setType(Comments::TransferOrderItem);

  _captive = FALSE;
  _dstwhsid	= -1;
  _itemsiteid	= -1;
  _transwhsid	= -1;
  _toheadid	= -1;
  _taxzoneid	= -1;
  adjustSize();
  
  _inventoryButton->setEnabled(_showAvailability->isChecked());
  _dependencyButton->setEnabled(_showAvailability->isChecked());
  _availability->setEnabled(_showAvailability->isChecked());
  _showIndented->setEnabled(_showAvailability->isChecked());
}

transferOrderItem::~transferOrderItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void transferOrderItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse transferOrderItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant  param;
  bool      valid;

  _prev->setEnabled(true);
  _next->setEnabled(true);
  _next->setText(tr("Next"));

  param = pParams.value("tohead_id", &valid);
  if (valid)
    _toheadid = param.toInt();

  param = pParams.value("srcwarehouse_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("taxzone_id", &valid);
  if (valid)
    _taxzoneid = param.toInt();

  param = pParams.value("orderNumber", &valid);
  if (valid)
    _orderNumber->setText(param.toString());

  param = pParams.value("curr_id", &valid);
  if (valid)
    _freight->setId(param.toInt());

  param = pParams.value("orderDate", &valid);
  if (valid)
    _freight->setEffective(param.toDate());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _save->setEnabled(FALSE);
      _next->setText(tr("New"));
      _comments->setEnabled(FALSE);
      _item->setReadOnly(false);
      _item->setFocus();

      _item->addExtraClause(QString("(item_id IN ("
				    "  SELECT itemsite_item_id"
				    "  FROM itemsite"
				    "  WHERE itemsite_warehous_id=%1))")
				    .arg(_warehouse->id()) );

      prepare();

      param = pParams.value("captive", &valid);
      if (valid)
        _captive = TRUE;

      param = pParams.value("item_id", &valid);
      if (valid)
        _item->setId(param.toInt());

      param = pParams.value("dueDate", &valid);
      if (valid)
        _scheduledDate->setDate(param.toDate());

      param = pParams.value("qty", &valid);
      if (valid)
        _qtyOrdered->setDouble(param.toDouble());

      q.prepare("SELECT count(*) AS cnt"
                "  FROM toitem"
                " WHERE (toitem_tohead_id=:tohead_id);");
      q.bindValue(":tohead_id", _toheadid);
      q.exec();
      if(!q.first() || q.value("cnt").toInt() == 0)
        _prev->setEnabled(false);
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(true);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
    }
  }

  if(cView == _mode)
  {
    _item->setReadOnly(true);
    _qtyOrdered->setEnabled(false);
    _freight->setEnabled(false);
    _scheduledDate->setEnabled(false);
    _notes->setEnabled(false);
    _comments->setReadOnly(true);
    _itemcharView->setEnabled(false);
    _promisedDate->setEnabled(false);

    _save->hide();

    _close->setFocus();
  }

  param = pParams.value("toitem_id", &valid);
  if (valid)
  {
    _toitemid = param.toInt();

    q.prepare("SELECT a.toitem_id AS id"
	      "  FROM toitem AS a, toitem AS b"
	      " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	      "   AND  (b.toitem_id=:id))"
	      " ORDER BY a.toitem_linenumber "
	      " LIMIT 1;");
    q.bindValue(":id", _toitemid);
    q.exec();
    if(!q.first() || q.value("id").toInt() == _toitemid)
      _prev->setEnabled(false);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    q.prepare("SELECT a.toitem_id AS id"
	      "  FROM toitem AS a, toitem AS b"
	      " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	      "   AND  (b.toitem_id=:id))"
	      " ORDER BY a.toitem_linenumber DESC"
	      " LIMIT 1;");
    q.bindValue(":id", _toitemid);
    q.exec();
    if(q.first() && q.value("id").toInt() == _toitemid)
    {
      if(cView == _mode)
        _next->setEnabled(false);
      else
        _next->setText(tr("New"));
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  populate();	// TODO: should this go BEFORE pParams.value("item_id")?

  _modified = false;

  return NoError;
}

void transferOrderItem::prepare()
{
  if (_mode == cNew)
  {
    q.prepare( "SELECT (COALESCE(MAX(toitem_linenumber), 0) + 1) AS _linenumber "
               "FROM toitem "
               "WHERE (toitem_tohead_id=:toitem_id)" );
    q.bindValue(":toitem_id", _toheadid);
    q.exec();
    if (q.first())
      _lineNumber->setText(q.value("_linenumber").toString());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MIN(toitem_schedshipdate) AS scheddate "
               "FROM toitem "
               "WHERE (toitem_tohead_id=:tohead_id);" );
    q.bindValue(":tohead_id", _toheadid);
    q.exec();
    if (q.first())
      _scheduledDate->setDate(q.value("scheddate").toDate());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  _modified = false;
}

void transferOrderItem::clear()
{
  _item->setId(-1);
  _qtyOrdered->clear();
  _freight->reset();
  _scheduledDate->clear();
  _promisedDate->clear();
  _stdcost->clear();
  _shippedToDate->clear();
  _onHand->clear();
  _allocated->clear();
  _unallocated->clear();
  _onOrder->clear();
  _available->clear();
  _itemchar->removeRows(0, _itemchar->rowCount());
  _notes->clear();
  _comments->setId(-1);
  _warehouse->setId(_preferences->value("PreferredWarehouse").toInt());
  _originalQtyOrd = 0.0;
  _modified = false;
}

void transferOrderItem::sSave()
{
  _save->setFocus();

  _error = true;
  if (!(_qtyOrdered->toDouble() > 0))
  {
    QMessageBox::warning( this, tr("Cannot Save Transfer Order Item"),
                          tr("<p>You must enter a valid Quantity Ordered "
			     "before saving this Transfer Order Item.")  );
    _qtyOrdered->setFocus();
    return;
  }

  if (!(_scheduledDate->isValid()))
  {
    QMessageBox::warning( this, tr("Cannot Save Transfer Order Item"),
                          tr("<p>You must enter a valid Schedule Date before "
			     "saving this Transfer Order Item.") );
    _scheduledDate->setFocus();
    return;
  }

  if (_qtyOrdered->toDouble() < _shippedToDate->toDouble())
  {
    QMessageBox::warning(this, tr("Cannot Save Transfer Order Item"),
			 tr("<p>You cannot set the quantity of the order to "
			    "a value less than the quantity that has already "
			    "been shipped."));
    _qtyOrdered->setFocus();
    return;
  }

  _error = false;

  QDate promiseDate;

  if (_metrics->boolean("UsePromiseDate"))
  {
    if (_promisedDate->isValid())
    {
      if (_promisedDate->isNull())
        promiseDate = omfgThis->endOfTime();
      else
        promiseDate = _promisedDate->date();
    }
  }
  else
    promiseDate = omfgThis->endOfTime();

  if (_mode == cNew || _mode == cEdit)
  {
    if (itemSite::createItemSite(this, _itemsiteid, _transwhsid, false) < 0 ||
	itemSite::createItemSite(this, _itemsiteid, _dstwhsid, true) < 0)
      return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('toitem_toitem_id_seq') AS toitem_id");
    if (q.first())
      _toitemid = q.value("toitem_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      reject();
      return;
    }

    q.prepare( "INSERT INTO toitem "
               "( toitem_id, toitem_tohead_id, toitem_linenumber,"
               "  toitem_item_id, toitem_status, toitem_duedate,"
               "  toitem_schedshipdate, toitem_schedrecvdate,"
               "  toitem_qty_ordered, toitem_notes,"
               "  toitem_uom, toitem_stdcost, toitem_prj_id,"
               "  toitem_freight, toitem_freight_curr_id ) "
               "VALUES "
	             "( :toitem_id, :toitem_tohead_id, :toitem_linenumber,"
               "  :toitem_item_id, 'U', :toitem_duedate,"
	             "  :toitem_schedshipdate, :toitem_schedrecvdate,"
               "  :toitem_qty_ordered, :toitem_notes,"
               "  :toitem_uom, stdCost(:toitem_item_id), :toitem_prj_id,"
               "  :toitem_freight, :toitem_freight_curr_id );" );

  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE toitem SET"
               "  toitem_duedate=:toitem_duedate,"
	             "  toitem_schedshipdate=:toitem_schedshipdate,"
	             "  toitem_schedrecvdate=:toitem_schedrecvdate,"
               "  toitem_qty_ordered=:toitem_qty_ordered,"
	             "  toitem_notes=:toitem_notes,"
	             "  toitem_prj_id=:toitem_prj_id,"
               "  toitem_freight=:toitem_freight,"
	             "  toitem_freight_curr_id=:toitem_freight_curr_id,"
	             "  toitem_lastupdated=CURRENT_TIMESTAMP "
               "WHERE (toitem_id=:toitem_id);" );
  }

  q.bindValue(":toitem_id",		  _toitemid);
  q.bindValue(":toitem_tohead_id",	  _toheadid);
  q.bindValue(":toitem_linenumber",	  _lineNumber->text().toInt());
  q.bindValue(":toitem_item_id",	  _item->id());
  q.bindValue(":toitem_duedate",	  _scheduledDate->date());
  q.bindValue(":toitem_schedshipdate",    _scheduledDate->date()); // ??
  q.bindValue(":toitem_schedrecvdate",    promiseDate);
  q.bindValue(":toitem_qty_ordered",	  _qtyOrdered->toDouble());
  q.bindValue(":toitem_notes",		  _notes->toPlainText());
  q.bindValue(":toitem_uom",		  _item->uom());
  // TODO: q.bindValue(":toitem_prj_id",		);
  q.bindValue(":toitem_freight",	  _freight->localValue());
  q.bindValue(":toitem_freight_curr_id",  _freight->id());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode != cView)
  {
    QString type = "TI";

    q.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value) AS result;");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_type", type);
      q.bindValue(":target_id", _toitemid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
      if (q.first())
      {
	      int result = q.value("result").toInt();
	      if (result < 0)
	      {
	        systemError(this, storedProcErrorLookup("updateCharAssignment", result),
		      __FILE__, __LINE__);
	        return;
	      }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
      }
    }
  }
  
  _modified = false;
  omfgThis->sTransferOrdersUpdated(_toheadid);

  if ((!_captive) && (!_canceling) && (cNew == _mode))
  {
    clear();
    prepare();
    _prev->setEnabled(true);
    _item->setFocus();
  }
  else
    close();
}

void transferOrderItem::sPopulateItemInfo(int pItemid)
{
  _itemchar->removeRows(0, _itemchar->rowCount());
  if (pItemid != -1)
  {
    q.prepare("SELECT stdcost(:item_id) AS stdcost, itemsite_id "
	      "FROM itemsite "
	      "WHERE ((itemsite_item_id=:item_id)"
	      "  AND  (itemsite_warehous_id=:whsid));");
    q.bindValue(":item_id", pItemid);
    q.bindValue(":whsid", _warehouse->id());
    q.exec();
    if (q.first())
    {
      _stdcost->setBaseValue(q.value("stdcost").toDouble());
      _itemsiteid = q.value("itemsite_id").toInt();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value,"
	       "               (SELECT c.charass_value"
	       "                FROM charass c"
	       "                WHERE ((c.charass_target_type='I')"
	       "                  AND  (c.charass_target_id=:item_id)"
	       "                  AND  (c.charass_default)"
	       "                  AND  (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type=:totype"
               "      AND b.charass_target_id=:toitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":totype", "TI");
    q.bindValue(":toitem_id", _toitemid);
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
      _itemchar->setData(idx, pItemid, Xt::IdRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrderItem::sDetermineAvailability()
{
  if(  (_item->id()==_availabilityLastItemid)
    && (_warehouse->id()==_availabilityLastWarehousid)
    && (_scheduledDate->date()==_availabilityLastSchedDate)
    && (_showAvailability->isChecked()==_availabilityLastShow)
    && (_showIndented->isChecked()==_availabilityLastShowIndent)
    && (_qtyOrdered->toDouble()==_availabilityQtyOrdered) )
    return;

  _availabilityLastItemid	= _item->id();
  _availabilityLastWarehousid	= _warehouse->id();
  _availabilityLastSchedDate	= _scheduledDate->date();
  _availabilityLastShow		= _showAvailability->isChecked();
  _availabilityLastShowIndent	= _showIndented->isChecked();
  _availabilityQtyOrdered	= _qtyOrdered->toDouble();

  _availability->clear();

  if ((_item->isValid()) && (_scheduledDate->isValid()) && (_showAvailability->isChecked()) )
  {
    XSqlQuery availability;
    availability.prepare( "SELECT itemsite_id,"
                          "       qoh,"
                          "       allocated,"
                          "       noNeg(qoh - allocated) AS unallocated,"
                          "       ordered,"
                          "       (qoh - allocated + ordered) AS available "
                          "FROM ( SELECT itemsite_id, itemsite_qtyonhand AS qoh,"
                          "              qtyAllocated(itemsite_id, DATE(:date)) AS allocated,"
                          "              qtyOrdered(itemsite_id, DATE(:date)) AS ordered "
                          "       FROM itemsite, item "
                          "       WHERE ((itemsite_item_id=item_id)"
                          "        AND (item_id=:item_id)"
                          "        AND (itemsite_warehous_id=:warehous_id)) ) AS data;" );
    availability.bindValue(":date", _scheduledDate->date());
    availability.bindValue(":item_id", _item->id());
    availability.bindValue(":warehous_id", _warehouse->id());
    availability.exec();
    if (availability.first())
    {
      _onHand->setText(availability.value("qoh").toString());
      _allocated->setText(availability.value("allocated").toString());
      _unallocated->setText(availability.value("unallocated").toString());
      _onOrder->setText(availability.value("ordered").toString());
      _available->setText(availability.value("available").toString());

      QString stylesheet;
      if (availability.value("available").toDouble() < _qtyOrdered->toDouble())
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
      _available->setStyleSheet(stylesheet);

      if ( (_item->itemType() == "M") )
      {
        if(_showIndented->isChecked())
        {
          availability.prepare("SELECT indentedBOM(:item_id) AS bomwork_set_id;");
          availability.bindValue(":item_id", _item->id());
          availability.exec();
          if(availability.first())
          {
            int _worksetid = availability.value("bomwork_set_id").toInt();
	    if (_worksetid < 0)
	    {
	      systemError(this, storedProcErrorLookup("indentedBOM", _worksetid),
			  __FILE__, __LINE__);
	      return;
	    }
            availability.prepare("SELECT bomwork_id, *, "
                                 "       bomwork_seqnumber AS bomitem_seqnumber,"
                                 "       totalalloc + pendalloc AS totalalloc,"
                                 "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
                                 "       bomwork_level - 1 AS xtindentrole,"
                                 "       'qty' AS totalalloc_xtnumericrole,"
                                 "       'qty' AS totalavail_xtnumericrole,"
                                 "       'qty' AS pendalloc_xtnumericrole,"
                                 "       'qty' AS ordered_xtnumericrole,"
                                 "       'qty' AS qoh_xtnumericrole,"
                                 "       CASE WHEN (qoh < pendalloc) THEN 'error'"
                                 "       END AS qoh_qtforegroundrole,"
                                 "       CASE WHEN (qoh + ordered - (totalalloc + pendalloc) < 0) THEN 'error'"
                                 "            WHEN (qoh + ordered - (totalalloc + pendalloc) < reorderlevel) THEN 'warning'"
                                 "       END AS totalavail_qtforegroundrole "
                                 "  FROM ( SELECT itemsite_id AS itemsiteid,"
                                 "                CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                                 "                bomwork_id, bomwork_parent_id,"
                                 "                bomwork_level, bomwork_seqnumber,"
                                 "                item_number, uom_name,"
                                 "                (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
                                 "                ((bomwork_qtyfxd + bomwork_qtyper * :qty) * (1 + bomwork_scrap)) AS pendalloc,"
                                 "                (qtyAllocated(itemsite_id, DATE(:schedDate)) - ((bomwork_qtyfxd + bomwork_qtyper * :origOrdQty) * (1 + bomwork_scrap))) AS totalalloc,"
                                 "                noNeg(itemsite_qtyonhand) AS qoh,"
                                 "                qtyOrdered(itemsite_id, DATE(:schedDate)) AS ordered"
                                 "           FROM bomwork, item, itemsite, uom"
                                 "          WHERE ( (itemsite_item_id=item_id)"
                                 "            AND   (itemsite_warehous_id=:warehous_id)"
                                 "            AND   (bomwork_item_id=item_id)"
                                 "            AND   (item_inv_uom_id=uom_id)"
                                 "            AND   (bomwork_set_id=:bomwork_set_id)"
                                 "                )"
                                 "       ) AS data "
                                 "ORDER BY bomworksequence(bomwork_id);");
            availability.bindValue(":bomwork_set_id", _worksetid);
            availability.bindValue(":warehous_id",    _warehouse->id());
            availability.bindValue(":qty",            _qtyOrdered->toDouble());
            availability.bindValue(":schedDate",      _scheduledDate->date());
            availability.bindValue(":origQtyOrd",     _originalQtyOrd);
            availability.exec();
            _availability->populate(availability);
	    if (availability.lastError().type() != QSqlError::NoError)
	    {
	      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }

            //  All done with the bomwork set, delete it
            availability.prepare("SELECT deleteBOMWorkset(:bomwork_set_id) AS result;");
            availability.bindValue(":bomwork_set_id", _worksetid);
            availability.exec();
	    if (availability.first())
	    {
	      int result = availability.value("result").toInt();
	      if (result < 0)
	      {
		systemError(this, storedProcErrorLookup("deleteBOMWorkset", result),
			    __FILE__, __LINE__);
		return;
	      }
	    }
	    else if (availability.lastError().type() != QSqlError::NoError)
	    {
	      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
          }
        }
        else
        {
          int itemsiteid = availability.value("itemsite_id").toInt();
          availability.prepare( "SELECT itemsiteid, *, "
                                "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
                                "       'qty' AS totalalloc_xtnumericrole,"
                                "       'qty' AS totalavail_xtnumericrole,"
                                "       'qty' AS pendalloc_xtnumericrole,"
                                "       'qty' AS ordered_xtnumericrole,"
                                "       'qty' AS qoh_xtnumericrole,"
                                "       CASE WHEN (qoh < pendalloc) THEN 'error'"
                                "       END AS qoh_qtforegroundrole,"
                                "       CASE WHEN (qoh + ordered - (totalalloc + pendalloc) < 0) THEN 'error'"
                                "            WHEN (qoh + ordered - (totalalloc + pendalloc) < reorderlevel) THEN 'warning'"
                                "       END AS totalavail_qtforegroundrole "
                                "FROM ( SELECT cs.itemsite_id AS itemsiteid,"
                                "              CASE WHEN(cs.itemsite_useparams) THEN cs.itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                                "              bomitem_seqnumber, item_number,"
                                "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
                                "              itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, (bomitem_qtyfxd + bomitem_qtyper * :qty) * (1 + bomitem_scrap)) AS pendalloc,"
                                "              (qtyAllocated(cs.itemsite_id, DATE(:schedDate)) - itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, (bomitem_qtyfxd + bomitem_qtyper * :origQtyOrd) * (1 + bomitem_scrap))) AS totalalloc,"
                                "              noNeg(cs.itemsite_qtyonhand) AS qoh,"
                                "              qtyOrdered(cs.itemsite_id, DATE(:schedDate)) AS ordered "
                                "       FROM itemsite AS cs, itemsite AS ps, item, bomitem, uom "
                                "       WHERE ( (bomitem_item_id=item_id)"
                                "        AND (item_inv_uom_id=uom_id)"
                                "        AND (cs.itemsite_item_id=item_id)"
                                "        AND (cs.itemsite_warehous_id=ps.itemsite_warehous_id)"
                                "        AND (bomitem_parent_item_id=ps.itemsite_item_id)"
		                "        AND (bomitem_rev_id=getActiveRevId('BOM',bomitem_parent_item_id))"
                                "        AND (:schedDate BETWEEN bomitem_effective AND (bomitem_expires-1))"
                                "        AND (ps.itemsite_id=:itemsite_id) ) ) AS data "
                                "ORDER BY bomitem_seqnumber;" );
          availability.bindValue(":itemsite_id", itemsiteid);
          availability.bindValue(":qty", _qtyOrdered->toDouble());
          availability.bindValue(":schedDate", _scheduledDate->date());
          availability.bindValue(":origQtyOrd", _originalQtyOrd);
          availability.exec();
          _availability->populate(availability);
          if (availability.lastError().type() != QSqlError::NoError)
          {
            systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
      }
      else
        _availability->setEnabled(FALSE);
    }
    else if (availability.lastError().type() != QSqlError::NoError)
    {
      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _onHand->clear();
    _allocated->clear();
    _unallocated->clear();
    _onOrder->clear();
    _available->clear();
  }
}

void transferOrderItem::populate()
{
  XSqlQuery item;

  if (_mode == cNew)
  {
    item.prepare("SELECT tohead.*, warehous_id, warehous_code "
		 "FROM whsinfo, tohead "
		 "WHERE ((tohead_src_warehous_id=warehous_id)"
		 "  AND  (tohead_id=:id));") ;
    item.bindValue(":id", _toheadid);
  }
  else
  {
    item.prepare("SELECT toitem_linenumber,toitem_status,toitem_qty_ordered, "
                 "       toitem_schedshipdate,toitem_notes,toitem_schedrecvdate,"
                 "       toitem_freight,toitem_item_id, toitem_status,"
                 "       tohead_id,tohead_taxzone_id,tohead_trns_warehous_id,"
                 "       tohead_dest_warehous_id,tohead_number, "
		 "       warehous_id, warehous_code,"
		 "       stdCost(toitem_item_id) AS stdcost,"
		 "       itemsite_id,"
		 "       (SELECT COALESCE(SUM(shipitem_qty), 0)"
		 "          FROM shipitem, shiphead"
		 "         WHERE ((shipitem_shiphead_id=shiphead_id)"
		 "           AND  (shiphead_order_type='TO')"
		 "           AND  (shipitem_orderitem_id=toitem_id))) AS shipitem_qty, "
     "        SUM(taxhist_tax) AS tax "
		 "FROM whsinfo, tohead, itemsite, toitem  "
     " LEFT OUTER JOIN toitemtax ON (toitem_id=taxhist_parent_id) "
		 "WHERE ((toitem_tohead_id=tohead_id)"
		 "  AND  (tohead_src_warehous_id=warehous_id)"
		 "  AND  (itemsite_item_id=toitem_item_id)"
		 "  AND  (itemsite_warehous_id=tohead_src_warehous_id)"
		 "  AND  (toitem_id=:id)) "
     "GROUP BY toitem_linenumber,toitem_status,toitem_qty_ordered, "
     "       toitem_schedshipdate,toitem_notes,toitem_schedrecvdate,"
     "       tohead_id,tohead_taxzone_id,tohead_trns_warehous_id,"
     "       tohead_dest_warehous_id,tohead_number,toitem_freight, "
		 "       warehous_id, warehous_code,toitem_item_id,"
		 "       toitem_status, stdcost, shipitem_qty, itemsite_id;");
    item.bindValue(":id", _toitemid);
  }

  item.exec();
  if (item.first())
  {
    _toheadid	= item.value("tohead_id").toInt();
    _taxzoneid	= item.value("tohead_taxzone_id").toInt();
    _transwhsid	= item.value("tohead_trns_warehous_id").toInt();
    _dstwhsid	= item.value("tohead_dest_warehous_id").toInt();
    _orderNumber->setText(item.value("tohead_number").toString());

    _warehouse->clear();
    _warehouse->append(item.value("warehous_id").toInt(),
		       item.value("warehous_code").toString());

    if (_mode == cEdit || _mode == cView)
    {
      _itemsiteid	= item.value("itemsite_id").toInt();
      _comments->setId(_toitemid);
      _lineNumber->setText(item.value("toitem_linenumber").toString());
      _stdcost->setBaseValue(item.value("stdcost").toDouble());
      _shippedToDate->setDouble(item.value("shipitem_qty").toDouble());

      // do tax stuff before _qtyOrdered so signal cascade has data to work with
      _qtyOrdered->setDouble(item.value("toitem_qty_ordered").toDouble());
      _scheduledDate->setDate(item.value("toitem_schedshipdate").toDate());
      _notes->setText(item.value("toitem_notes").toString());
      if (!item.value("toitem_schedrecvdate").isNull() && _metrics->boolean("UsePromiseDate"))
	      _promisedDate->setDate(item.value("toitem_schedrecvdate").toDate());
      _originalQtyOrd	= _qtyOrdered->toDouble();

      //  Set the _item here to tickle signal cascade
      _item->setId(item.value("toitem_item_id").toInt());
      _freight->setLocalValue(item.value("toitem_freight").toDouble());
      _tax->setLocalValue(item.value("tax").toDouble());
      sDetermineAvailability();
    }
  }
  else if (item.lastError().type() != QSqlError::NoError)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if( (cNew == _mode) || ((cEdit == _mode) &&
			  (item.value("toitem_status").toString() == "O")) )
    _cancel->setEnabled(_shippedToDate->toDouble()==0.0);
  else
    _cancel->setEnabled(false);
}

void transferOrderItem::sNext()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved! "
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  clear();
  prepare();
  _item->setFocus();

  if(cNew == _mode)
  {
    _modified = false;
    return;
  }

  q.prepare("SELECT a.toitem_id AS id"
	    "  FROM toitem AS a, toitem AS b"
	    " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	    "   AND  (a.toitem_linenumber > b.toitem_linenumber)"
	    "   AND  (b.toitem_id=:id))"
	    " ORDER BY a.toitem_linenumber"
	    " LIMIT 1;");
  q.bindValue(":id", _toitemid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("toitem_id", q.value("id").toInt());

    if(cNew == _mode || cEdit == _mode)
      params.append("mode", "edit");
    else
      params.append("mode", "view");

    set(params);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (cView != _mode)
  {
    ParameterList params;
    params.append("tohead_id", _toheadid);
    params.append("mode", "new");
    set(params);
  }
}


void transferOrderItem::sPrev()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved! "
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  clear();
  prepare();
  _item->setFocus();


  if(cNew == _mode)
    q.prepare("SELECT toitem_id AS id"
	      "  FROM toitem"
	      " WHERE (toitem_tohead_id=:tohead_id)"
	      " ORDER BY toitem_linenumber DESC"
	      " LIMIT 1;");
  else
    q.prepare("SELECT a.toitem_id AS id"
	      "  FROM toitem AS a, toitem AS b"
	      " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	      "   AND  (a.toitem_linenumber < b.toitem_linenumber)"
	      "   AND  (b.toitem_id=:id))"
	      " ORDER BY a.toitem_linenumber DESC"
	      " LIMIT 1;");

  q.bindValue(":id", _toitemid);
  q.bindValue(":tohead_id", _toheadid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("toitem_id", q.value("id").toInt());

    if(cNew == _mode || cEdit == _mode)
      params.append("mode", "edit");
    else
      params.append("mode", "view");

    set(params);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrderItem::sChanged()
{
  _modified = true;
}

void transferOrderItem::reject()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved! "
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }
  XDialog::reject();
}

void transferOrderItem::sCancel()
{
  _canceling = true;
  
  sSave();
  if(_error) 
    return;

  q.prepare("UPDATE toitem SET toitem_status='X' WHERE (toitem_id=:toitem_id);");
  q.bindValue(":toitem_id", _toitemid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _modified = false;
  _canceling = false;
  omfgThis->sTransferOrdersUpdated(_toheadid);
  if (cNew == _mode)
  {
    clear();
    prepare();
    _prev->setEnabled(true);
    _item->setFocus();
  }
  else
    close();

}

void transferOrderItem::sCalculateTax()
{  
  XSqlQuery calcq;
  calcq.prepare("SELECT ROUND(calculateTax(tohead_taxzone_id,getFreightTaxTypeId(),tohead_orderdate,tohead_freight_curr_id,:freight),2) AS tax "
                "FROM tohead "
                "WHERE (tohead_id=:tohead_id); " );

  calcq.bindValue(":tohead_id", _toheadid);
  calcq.bindValue(":freight", _freight->localValue());
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrderItem::sTaxDetail()
{
  XSqlQuery fid;
  fid.exec("SELECT getFreightTaxTypeId() AS taxtype_id;");
  fid.first();
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id",   _taxzoneid);
  params.append("taxtype_id",  fid.value("taxtype_id").toInt());
  params.append("date", _tax->effective());
  params.append("curr_id", _tax->id());
  params.append("subtotal", _freight->localValue());
  params.append("readOnly");

  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void transferOrderItem::sHandleButton()
{
  if (_inventoryButton->isChecked())
    _availabilityStack->setCurrentIndex(0);
  else
    _availabilityStack->setCurrentIndex(1);
}
