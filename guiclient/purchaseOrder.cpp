/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purchaseOrder.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "comment.h"
#include "itemSourceList.h"
#include "mqlutil.h"
#include "poitemTableModel.h"
#include "printPurchaseOrder.h"
#include "purchaseOrderItem.h"
#include "vendorAddressList.h"
#include "taxBreakdown.h"
#include "salesOrder.h"
#include "workOrder.h"
#include "openPurchaseOrder.h" 

#define cDelete 0x01
#define cClose  0x02

purchaseOrder::purchaseOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _so->setNameVisible(false);
  _so->setDescriptionVisible(false);

  _orderNumber->setValidator(new QIntValidator(this));

  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sCalculateTotals()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_orderDate,   SIGNAL(newDate(QDate)), this, SLOT(sHandleOrderDate()));
  connect(_orderNumber, SIGNAL(lostFocus()), this, SLOT(sHandleOrderNumber()));
  connect(_orderNumber, SIGNAL(textChanged(const QString&)), this, SLOT(sSetUserOrderNumber()));
  connect(_poCurrency,  SIGNAL(newID(int)),     this, SLOT(sCurrencyChanged()));
  connect(_poitem, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleDeleteButton()));
  connect(_purchaseOrderInformation, SIGNAL(currentChanged(int)), this, SLOT(sTabChanged(int)));
  connect(_qecurrency,  SIGNAL(newID(int)),     this, SLOT(sCurrencyChanged()));
  connect(_qedelete,    SIGNAL(clicked()),      this, SLOT(sQEDelete()));
  connect(_qesave,      SIGNAL(clicked()),      this, SLOT(sQESave()));
  connect(_save,        SIGNAL(clicked()),      this, SLOT(sSave()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_tax,          SIGNAL(valueChanged()), this, SLOT(sCalculateTotals()));
  connect(_taxZone,      SIGNAL(newID(int)), this, SLOT(sTaxZoneChanged()));
  connect(_vendaddrList, SIGNAL(clicked()),     this, SLOT(sVendaddrList()));
  connect(_vendor,       SIGNAL(newId(int)),    this, SLOT(sHandleVendor(int)));
  connect(_vendAddr,     SIGNAL(changed()),     _vendaddrCode, SLOT(clear()));
  connect(_warehouse,    SIGNAL(newID(int)),    this, SLOT(sHandleShipTo()));

  connect(_vendAddr, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _vendCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  connect(_shiptoAddr, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _shiptoCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

#ifndef Q_WS_MAC
  _vendaddrList->setMaximumWidth(25);
#endif

  _poCurrency->setLabel(_poCurrencyLit);

  _poitem->addColumn(tr("#"),              _whsColumn,    Qt::AlignCenter,true,  "poitem_linenumber");
  _poitem->addColumn(tr("Status"),         _statusColumn, Qt::AlignCenter,true,  "poitemstatus");
  _poitem->addColumn(tr("Item"),           _itemColumn,   Qt::AlignLeft,  true,  "item_number");
  _poitem->addColumn(tr("Description"),    -1,            Qt::AlignLeft,  true,  "item_descrip");
  _poitem->addColumn(tr("Orgl. Due Date"), _dateColumn,   Qt::AlignRight, false, "orgl_duedate");
  _poitem->addColumn(tr("Due Date Now"),   _dateColumn,   Qt::AlignRight, true,  "poitem_duedate");
  _poitem->addColumn(tr("Ordered"),        _qtyColumn,    Qt::AlignRight, true,  "poitem_qty_ordered");
  _poitem->addColumn(tr("Received"),       _qtyColumn,    Qt::AlignRight, false, "poitem_qty_received");
  _poitem->addColumn(tr("Returned"),       _qtyColumn,    Qt::AlignRight, false, "poitem_qty_returned");
  _poitem->addColumn(tr("Vouchered"),      _qtyColumn,    Qt::AlignRight, false, "poitem_qty_vouchered");
  _poitem->addColumn(tr("UOM"),            _uomColumn,    Qt::AlignCenter,true,  "poitem_vend_uom");
  _poitem->addColumn(tr("Unit Price"),     _priceColumn,  Qt::AlignRight, true,  "poitem_unitprice");
  _poitem->addColumn(tr("Ext. Price"),     _moneyColumn,  Qt::AlignRight, true,  "extprice");
  _poitem->addColumn(tr("Freight"),        _moneyColumn,  Qt::AlignRight, false, "poitem_freight");
  _poitem->addColumn(tr("Freight Recv."),  _moneyColumn,  Qt::AlignRight, false, "poitem_freight_received");
  _poitem->addColumn(tr("Freight Vchr."),  _moneyColumn,  Qt::AlignRight, false, "poitem_freight_vouchered");
  _poitem->addColumn(tr("Std. Cost"),      _moneyColumn,  Qt::AlignRight, false, "poitem_stdcost");
  _poitem->addColumn(tr("Vend. Item#"),    _itemColumn,   Qt::AlignCenter,false, "poitem_vend_item_number");
  _poitem->addColumn(tr("Manufacturer"),   _itemColumn,   Qt::AlignRight, false, "poitem_manuf_name");
  _poitem->addColumn(tr("Manuf. Item#"),   _itemColumn,   Qt::AlignRight, false, "poitem_manuf_item_number");
  _poitem->addColumn(tr("Demand Type"),    _itemColumn,   Qt::AlignCenter,false, "demand_type");
  _poitem->addColumn(tr("Order"),          _itemColumn,   Qt::AlignRight, false, "order_number");

  _qeitem = new PoitemTableModel(this);
  _qeitemView->setModel(_qeitem);
  _qeitem->select();

  q.exec("SELECT usr_id "
         "FROM usr "
         "WHERE (usr_username=getEffectiveXtUser());");
  if(q.first())
    _agent->setId(q.value("usr_id").toInt());

  _captive         = false;
  _userOrderNumber = false;
  _printed         = false;

  setPoheadid(-1);
  _vendaddrid = -1;

  _cachedTabIndex = 0;

  _mode = cView;        // initialize _mode to something safe - bug 3768
 
  _printPO->setChecked(_metrics->boolean("DefaultPrintPOOnSave"));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _receivingWhseLit->hide();
    _warehouse->hide();
  }

  if (_metrics->boolean("EnableDropShipments"))
    _dropShip->setEnabled(FALSE);
  else
    _dropShip->hide();
  _so->setReadOnly(TRUE);
}

void purchaseOrder::setPoheadid(const int pId)
{
  _poheadid = pId;
  _qeitem->setHeadId(pId);
}

purchaseOrder::~purchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void purchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse purchaseOrder::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      itemsiteid;
  int      parentwo = -1;
  int      parentso = -1;
  int      prjid = -1;
  double   qty;
  QDate    dueDate;
  QString  prnotes;

  setPoheadid(-1);
  int _prid = -1;
  param = pParams.value("pr_id", &valid);
  if (valid)
    _prid = param.toInt();

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    itemsiteid = param.toInt();
  else
    itemsiteid = -1;

  param = pParams.value("qty", &valid);
  if (valid)
    qty = param.toDouble();
  else
    qty = 0.0;

  param = pParams.value("dueDate", &valid);
  if (valid)
    dueDate = param.toDate();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "releasePr") )
    {
      _mode = cNew;

      if (param.toString() == "releasePr")
      {
        q.prepare( "SELECT pr_itemsite_id, pr_qtyreq, pr_duedate,"
                   "       CASE WHEN(pr_order_type='W') THEN"
                   "              COALESCE((SELECT womatl_wo_id FROM womatl WHERE womatl_id=pr_order_id),-1)"
                   "            ELSE -1"
                   "       END AS parentwo,"
                   "       CASE WHEN(pr_order_type='S') THEN pr_order_id"
                   "            ELSE -1"
                   "       END AS parentso,"
                   "       pr_prj_id, pr_releasenote "
                   "FROM pr "
                   "WHERE (pr_id=:pr_id);" );
        q.bindValue(":pr_id", _prid);
        q.exec();
        if (q.first())
        {
          itemsiteid = q.value("pr_itemsite_id").toInt();
          qty = q.value("pr_qtyreq").toDouble();
          dueDate = q.value("pr_duedate").toDate();
          parentwo = q.value("parentwo").toInt();
          parentso = q.value("parentso").toInt();
          prjid = q.value("pr_prj_id").toInt();
          prnotes = q.value("pr_releasenote").toString();
        }
        else
        {
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
          return UndefinedError;
        }
      }

      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_vendor, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
      //_new->setEnabled(TRUE);
      int itemsrcid =-1;
      int openpoid =-1;
      if (itemsiteid != -1)
      {
              q.prepare( "SELECT itemsite_item_id, itemsrc_id, itemsrc_default "
                   "FROM itemsite, itemsrc "
                   "WHERE ( (itemsrc_item_id=itemsite_item_id)"
                   " AND (itemsite_id=:itemsite_id) ) "
                    "LIMIT 1;" );
        q.bindValue(":itemsite_id", itemsiteid);
        q.exec();
        if (!q.first())
        {
          QMessageBox::warning(omfgThis, tr("Cannot Create P/O"),
                               tr("<p>A Purchase Order cannot be automatically "
                                  "created for this Item as there are no Item "
                                  "Sources for it.  You must create one or "
                                  "more Item Sources for this Item before "
                                  "the application can automatically create "
                                  "Purchase Orders for it." ) );
          return UndefinedError;
        }
                if (q.first())
                {
                  XSqlQuery itemsrcdefault;
                  itemsrcdefault.prepare("SELECT itemsrc_id FROM itemsrc "
                                         "WHERE ((itemsrc_item_id=:item_id) AND ( itemsrc_default='TRUE')) "); 
          itemsrcdefault.bindValue(":item_id", q.value("itemsite_item_id").toInt());
                  itemsrcdefault.exec();
                  if (itemsrcdefault.first())
                  {
                    itemsrcid=(itemsrcdefault.value("itemsrc_id").toInt());
                  }
                  else
                  {
        ParameterList itemSourceParams;
        itemSourceParams.append("item_id", q.value("itemsite_item_id").toInt());
        itemSourceParams.append("qty", qty);
  
        itemSourceList newdlg(omfgThis, "", TRUE);
        newdlg.set(itemSourceParams);
        itemsrcid = newdlg.exec();
        if (itemsrcid == XDialog::Rejected)
        {
          deleteLater();
          return UndefinedError;
            }
                  }
        }

        q.prepare( "SELECT itemsrc_vend_id, vend_name  "
                   "from itemsrc left join vend on vend.vend_id = itemsrc.itemsrc_vend_id "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        q.bindValue(":itemsrc_id", itemsrcid);
        q.exec();
        if (!q.first())
        {
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
          return UndefinedError;
        }
        else
        {
          int vendid = q.value("itemsrc_vend_id").toInt();
          QString vendname = q.value("vend_name").toString();

          q.prepare( "SELECT pohead_id "
                     "FROM pohead "
                     "WHERE ( (pohead_status='U')"
                     " AND (pohead_vend_id=:vend_id) ) "
                     "ORDER BY pohead_number "
                     "LIMIT 1;" );
          q.bindValue(":vend_id", vendid);
          q.exec();
          if (q.first())
          {
            if(QMessageBox::question( this, tr("An Open Purchase Order Exists"),
                tr("An Open Purchase Order already exists for this Vendor.\n"
                   "Would you like to use this Purchase Order?\n"
                   "Click Yes to use the existing Purchase Order otherwise a new one will be created."),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
            {
              ParameterList openPurchaseOrderParams;
              openPurchaseOrderParams.append("vend_id", vendid);
              openPurchaseOrderParams.append("vend_name", vendname);
              openPurchaseOrder newdlg(omfgThis, "", TRUE);
              newdlg.set(openPurchaseOrderParams);
              openpoid = newdlg.exec();

              if (openpoid == XDialog::Rejected)
              {
                deleteLater();
                return UndefinedError;
              }
//  Use an existing pohead
              _mode = cEdit;

              setPoheadid(openpoid);
              _orderNumber->setEnabled(FALSE);
              _orderDate->setEnabled(FALSE);
              _vendor->setReadOnly(TRUE);
              populate();
            }
            else
            {
              _vendor->setId(vendid);
              createHeader();
            }
          }
          else
          {
//  Create a new pohead for the chosen vend
            _vendor->setId(vendid);
            createHeader();
          }

//  Start to create the new Poitem
          ParameterList newItemParams;
          newItemParams.append("mode", "new");
          newItemParams.append("pohead_id", _poheadid);
          newItemParams.append("itemsite_id", itemsiteid);
          newItemParams.append("itemsrc_id", itemsrcid);

          if (qty > 0.0)
            newItemParams.append("qty", qty);

          if (!dueDate.isNull())
            newItemParams.append("dueDate", dueDate);

          if (parentwo != -1)
            newItemParams.append("parentWo", parentwo);

          if (parentso != -1)
            newItemParams.append("parentSo", parentso);

          if (prjid != -1)
            newItemParams.append("prj_id", prjid);

          newItemParams.append("pr_releasenote", prnotes);

          purchaseOrderItem poItem(this, "", TRUE);
          poItem.set(newItemParams);
          if (poItem.exec() != XDialog::Rejected)
          {
            if(_mode == cEdit)
            {
              // check for another open window
              QWidgetList list = omfgThis->windowList();
              for(int i = 0; i < list.size(); i++)
              {
                QWidget * w = list.at(i);
                if (strcmp(w->metaObject()->className(), "purchaseOrder") == 0 && w != this)
                {
                  purchaseOrder *other = (purchaseOrder*)w;
                  if(_poheadid == other->_poheadid)
                  {
                    if(_prid != -1 && cEdit == other->_mode)
                    {
                      q.prepare("SELECT deletePr(:pr_id) AS _result;");
                      q.bindValue(":pr_id", _prid);
                      q.exec();
                      omfgThis->sPurchaseRequestsUpdated();
                    }
                    other->sFillList();
                    other->setFocus();
                    return UndefinedError;
                  }
                }
              }
            }
            sFillList();
          }
          else
            _prid = -1;
        }
      }
      else
//  This is a new P/O without a chosen Itemsite Line Item
        createHeader();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _orderNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _vendor->setReadOnly(TRUE);

      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _new->setEnabled(TRUE);

    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _orderNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
          _taxZone->setEnabled(FALSE);
      _agent->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _terms->setType(XComboBox::Terms);
      _vendor->setReadOnly(TRUE);
          _vendCntct->setEnabled(FALSE);
          _vendAddr->setEnabled(FALSE);
          _shiptoCntct->setEnabled(FALSE);
          _shiptoAddr->setEnabled(FALSE);
      _shipVia->setEnabled(FALSE);
      _fob->setEnabled(FALSE);
      _status->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _freight->setEnabled(FALSE);
      _tax->setEnabled(FALSE);
      _vendaddrList->hide();
      _purchaseOrderInformation->removeTab(_purchaseOrderInformation->indexOf(_quickEntryTab));
      _poCurrency->setEnabled(FALSE);
      _qeitemView->setEnabled(FALSE);
      _qesave->setEnabled(FALSE);
      _qedelete->setEnabled(FALSE);
      _qecurrency->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _documents->setReadOnly(TRUE);

      _delete->hide();
      _edit->setText(tr("&View"));
      _close->setText(tr("&Close"));
      _save->hide();

      connect(_poitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_poitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

      _close->setFocus();
    }
  }

  if (_prid != -1 && cNew != _mode)
  {
    q.prepare("SELECT deletePr(:pr_id) AS _result;");
    q.bindValue(":pr_id", _prid);
    q.exec();
    _prid = -1;
    omfgThis->sPurchaseRequestsUpdated();
  }

  if(_prid != -1)
    _pridList.append(_prid);

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    setPoheadid(param.toInt());
    populate();
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  return NoError;
}

void purchaseOrder::createHeader()
{
//  Determine the new PO Number
  if ( (_metrics->value("PONumberGeneration") == "A") ||
       (_metrics->value("PONumberGeneration") == "O") )
  {
    populateOrderNumber();
    _vendor->setFocus();
  }
  else
    _orderNumber->setFocus();

  q.exec("SELECT NEXTVAL('pohead_pohead_id_seq') AS pohead_id;");
  if (q.first())
    setPoheadid(q.value("pohead_id").toInt());
  else
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  // need to set at least the _order date before the INSERT
  _comments->setId(_poheadid);
  _documents->setId(_poheadid);
  _orderDate->setDate(omfgThis->dbDate(), true);
  _status->setCurrentIndex(0);
  _vendor->setShowInactive(false);

  q.prepare( "INSERT INTO pohead "
             "( pohead_id, pohead_number, pohead_status,"
             "  pohead_agent_username, pohead_vend_id, pohead_taxzone_id,"
             "  pohead_orderdate, pohead_curr_id, pohead_saved) "
             "VALUES "
             "( :pohead_id, :pohead_number, 'U',"
             "  :pohead_agent_username, :pohead_vend_id, :pohead_taxzone_id, "
             "  :pohead_orderdate, :pohead_curr_id, false );" );
  q.bindValue(":pohead_id", _poheadid);
  q.bindValue(":pohead_agent_username", _agent->currentText());
  if (!_orderNumber->text().isEmpty())
    q.bindValue(":pohead_number", _orderNumber->text());
  if (_vendor->isValid())
    q.bindValue(":pohead_vend_id", _vendor->id());
  if (_taxZone->isValid())
    q.bindValue(":pohead_taxzone_id", _taxZone->id());
  q.bindValue(":pohead_orderdate", _orderDate->date());
  q.bindValue(":pohead_curr_id", _poCurrency->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  // Populate Ship To contact and addresses for the Receiving Site
  sHandleShipTo();
}

void purchaseOrder::populate()
{
  XSqlQuery po;
  po.prepare( "SELECT pohead.*, COALESCE(pohead_warehous_id, -1) AS warehous_id,"
                          "       COALESCE(pohead_cohead_id, -1) AS cohead_id,"
              "       CASE WHEN (pohead_status='U') THEN 0"
                          "            WHEN (pohead_status='O') THEN 1"
                          "            WHEN (pohead_status='C') THEN 2"
                          "       END AS status,"
              "       COALESCE(pohead_terms_id, -1) AS terms_id,"
              "       COALESCE(pohead_vend_id, -1) AS vend_id,"
              "       COALESCE(vendaddr_id, -1) AS vendaddrid,"
                          "       vendaddr_code "
              "FROM pohead JOIN vendinfo ON (pohead_vend_id=vend_id)"
                          "     LEFT OUTER JOIN vendaddrinfo ON (pohead_vendaddr_id=vendaddr_id)"
                          "     LEFT OUTER JOIN cohead ON (pohead_cohead_id=cohead_id) "
              "WHERE (pohead_id=:pohead_id);" );
  po.bindValue(":pohead_id", _poheadid);
  po.exec();
  if (po.first())
  {
    _orderNumber->setText(po.value("pohead_number"));
    _warehouse->setId(po.value("warehous_id").toInt());
    _orderDate->setDate(po.value("pohead_orderdate").toDate(), true);
    if(po.value("pohead_released").isValid())                                                             
      _releaseDate->setDate(po.value("pohead_released").toDate(), true);
    _agent->setText(po.value("pohead_agent_username").toString());
    _status->setCurrentIndex(po.value("status").toInt());
    _printed = po.value("pohead_printed").toBool();
    _terms->setId(po.value("terms_id").toInt());
    _shipVia->setText(po.value("pohead_shipvia"));
    _fob->setText(po.value("pohead_fob"));
    _notes->setText(po.value("pohead_comments").toString());
        _so->setId(po.value("cohead_id").toInt());

        if ((po.value("cohead_id").toInt())!=-1)
        {
          _dropShip->setEnabled(TRUE);
          _dropShip->setChecked(po.value("pohead_dropship").toBool());
        }
        else
    {
          _dropShip->setChecked(FALSE);
      _dropShip->setEnabled(FALSE);
        }

    _vendaddrid = po.value("vendaddrid").toInt();
    
    _vendCntct->setId(po.value("pohead_vend_cntct_id").toInt());
    _vendCntct->setHonorific(po.value("pohead_vend_cntct_honorific").toString());
    _vendCntct->setFirst(po.value("pohead_vend_cntct_first_name").toString());
    _vendCntct->setMiddle(po.value("pohead_vend_cntct_middle").toString());
    _vendCntct->setLast(po.value("pohead_vend_cntct_last_name").toString());
    _vendCntct->setSuffix(po.value("pohead_vend_cntct_suffix").toString());
    _vendCntct->setPhone(po.value("pohead_vend_cntct_phone").toString());
    _vendCntct->setTitle(po.value("pohead_vend_cntct_title").toString());
    _vendCntct->setFax(po.value("pohead_vend_cntct_fax").toString());
    _vendCntct->setEmailAddress(po.value("pohead_vend_cntct_email").toString());

        _shiptoCntct->setId(po.value("pohead_shipto_cntct_id").toInt());
    _shiptoCntct->setHonorific(po.value("pohead_shipto_cntct_honorific").toString());
    _shiptoCntct->setFirst(po.value("pohead_shipto_cntct_first_name").toString());
    _shiptoCntct->setMiddle(po.value("pohead_shipto_cntct_middle").toString());
    _shiptoCntct->setLast(po.value("pohead_shipto_cntct_last_name").toString());
    _shiptoCntct->setSuffix(po.value("pohead_shipto_cntct_suffix").toString());
    _shiptoCntct->setPhone(po.value("pohead_shipto_cntct_phone").toString());
    _shiptoCntct->setTitle(po.value("pohead_shipto_cntct_title").toString());
    _shiptoCntct->setFax(po.value("pohead_shipto_cntct_fax").toString());
    _shiptoCntct->setEmailAddress(po.value("pohead_shipto_cntct_email").toString());

    disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        if (_vendaddrid == -1)
      _vendaddrCode->setText(tr("Main"));
        else
          _vendaddrCode->setText(po.value("vendaddr_code"));
        _vendAddr->setId(_vendaddrid);
    _vendAddr->setLine1(po.value("pohead_vendaddress1").toString());
    _vendAddr->setLine2(po.value("pohead_vendaddress2").toString());
    _vendAddr->setLine3(po.value("pohead_vendaddress3").toString());
    _vendAddr->setCity(po.value("pohead_vendcity").toString());
    _vendAddr->setState(po.value("pohead_vendstate").toString());
    _vendAddr->setPostalCode(po.value("pohead_vendzipcode").toString());
    _vendAddr->setCountry(po.value("pohead_vendcountry").toString());
        connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));

        _shiptoAddr->setId(po.value("pohead_shiptoddress_id").toInt());
    _shiptoAddr->setLine1(po.value("pohead_shiptoaddress1").toString());
    _shiptoAddr->setLine2(po.value("pohead_shiptoaddress2").toString());
    _shiptoAddr->setLine3(po.value("pohead_shiptoaddress3").toString());
    _shiptoAddr->setCity(po.value("pohead_shiptocity").toString());
    _shiptoAddr->setState(po.value("pohead_shiptostate").toString());
    _shiptoAddr->setPostalCode(po.value("pohead_shiptozipcode").toString());
    _shiptoAddr->setCountry(po.value("pohead_shiptocountry").toString());

    _comments->setId(_poheadid);
    _documents->setId(_poheadid);
    _vendor->setId(po.value("vend_id").toInt());
    _taxZone->setId(po.value("pohead_taxzone_id").toInt());
    _poCurrency->setId(po.value("pohead_curr_id").toInt());
    _freight->setLocalValue(po.value("pohead_freight").toDouble());
  }

  sFillList();
}

void purchaseOrder::sSave()
{
  _save->setFocus();

  if(_orderNumber->hasFocus())
    sHandleOrderNumber();

  if (_orderNumber->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
      tr("You may not save this Purchase Order until you have entered a valid Purchase Order Number.") );
    _orderNumber->setFocus();
    return;
  }

  if (!_vendor->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
                           tr("You may not save this Purchase Order until you have selected a Vendor." ) );

    _vendor->setFocus();
    return;
  }

  if (_qeitem->isDirty() && ! sQESave())
    return;

  q.prepare( "SELECT poitem_id " 
             "FROM poitem "
             "WHERE (poitem_pohead_id=:pohead_id) "
             "LIMIT 1;" );
  q.bindValue(":pohead_id", _poheadid);
  q.exec();
  if (!q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
                           tr( "You may not save this Purchase Order until you have created at least one Line Item for it." ) );

    _new->setFocus();
    return;
  }

  XSqlQuery postatus;
  postatus.prepare( "SELECT pohead_status "
                    "FROM pohead "
                    "WHERE (pohead_id=:pohead_id);" );
  postatus.bindValue(":pohead_id", _poheadid);
  postatus.exec();
  if (postatus.first())
  {
    if ((postatus.value("pohead_status") == "O") && (_status->currentIndex() == 0))
    {
          QMessageBox::critical( this, tr("Cannot Save Purchase Order"),
                             tr( "This Purchase Order has been released. You may not set its Status back to 'Unreleased'." ) );

      _status->setFocus();
      return;
    }
  }

  q.prepare( "UPDATE pohead "
             "SET pohead_warehous_id=:pohead_warehous_id, pohead_orderdate=:pohead_orderdate,"
             "    pohead_shipvia=:pohead_shipvia, pohead_taxzone_id=:pohead_taxzone_id,"
             "    pohead_freight=:pohead_freight,"
             "    pohead_fob=:pohead_fob, pohead_agent_username=:pohead_agent_username,"
             "    pohead_terms_id=:pohead_terms_id,"
             "    pohead_vendaddr_id=:pohead_vendaddr_id,"
             "    pohead_comments=:pohead_comments, "
             "    pohead_curr_id=:pohead_curr_id,"
             "    pohead_status=:pohead_status,"
             "    pohead_saved=true,"
             "    pohead_vend_cntct_id=:pohead_vend_cntct_id,"
             "    pohead_vend_cntct_honorific=:pohead_vend_cntct_honorific,"
             "    pohead_vend_cntct_first_name=:pohead_vend_cntct_first_name,"
             "    pohead_vend_cntct_middle=:pohead_vend_cntct_middle,"
             "    pohead_vend_cntct_last_name=:pohead_vend_cntct_last_name,"
             "    pohead_vend_cntct_suffix=:pohead_vend_cntct_suffix,"
             "    pohead_vend_cntct_phone=:pohead_vend_cntct_phone,"
             "    pohead_vend_cntct_title=:pohead_vend_cntct_title,"
             "    pohead_vend_cntct_fax=:pohead_vend_cntct_fax,"
             "    pohead_vend_cntct_email=:pohead_vend_cntct_email,"
             "    pohead_vendaddress1=:pohead_vendaddress1,"
             "    pohead_vendaddress2=:pohead_vendaddress2,"
             "    pohead_vendaddress3=:pohead_vendaddress3,"
             "    pohead_vendcity=:pohead_vendcity,"
             "    pohead_vendstate=:pohead_vendstate,"
             "    pohead_vendzipcode=:pohead_vendzipcode,"
             "    pohead_vendcountry=:pohead_vendcountry,"
             "    pohead_shipto_cntct_id=:pohead_shipto_cntct_id,"
             "    pohead_shipto_cntct_honorific=:pohead_shipto_cntct_honorific,"
             "    pohead_shipto_cntct_first_name=:pohead_shipto_cntct_first_name,"
             "    pohead_shipto_cntct_middle=:pohead_shipto_cntct_middle,"
             "    pohead_shipto_cntct_last_name=:pohead_shipto_cntct_last_name,"
             "    pohead_shipto_cntct_suffix=:pohead_shipto_cntct_suffix,"
             "    pohead_shipto_cntct_phone=:pohead_shipto_cntct_phone,"
             "    pohead_shipto_cntct_title=:pohead_shipto_cntct_title,"
             "    pohead_shipto_cntct_fax=:pohead_shipto_cntct_fax,"
             "    pohead_shipto_cntct_email=:pohead_shipto_cntct_email,"
             "    pohead_shiptoddress_id=:pohead_shiptoddress_id,"
             "    pohead_shiptoaddress1=:pohead_shiptoaddress1,"
             "    pohead_shiptoaddress2=:pohead_shiptoaddress2,"
             "    pohead_shiptoaddress3=:pohead_shiptoaddress3,"
             "    pohead_shiptocity=:pohead_shiptocity,"
             "    pohead_shiptostate=:pohead_shiptostate,"
             "    pohead_shiptozipcode=:pohead_shiptozipcode,"
             "    pohead_shiptocountry=:pohead_shiptocountry,"
             "    pohead_dropship=:pohead_dropship "
             "WHERE (pohead_id=:pohead_id);" );
  q.bindValue(":pohead_id", _poheadid);
  if (_warehouse->isValid())
    q.bindValue(":pohead_warehous_id", _warehouse->id());
  if (_taxZone->id() != -1) 
    q.bindValue(":pohead_taxzone_id", _taxZone->id());
  q.bindValue(":pohead_orderdate", _orderDate->date());
  q.bindValue(":pohead_shipvia", _shipVia->currentText());
  q.bindValue(":pohead_fob", _fob->text());
  q.bindValue(":pohead_agent_username", _agent->currentText());
  if (_terms->isValid())
    q.bindValue(":pohead_terms_id", _terms->id());
  if (_vendaddrid != -1)
    q.bindValue(":pohead_vendaddr_id", _vendaddrid);
  q.bindValue(":pohead_comments", _notes->toPlainText());
  if (_vendCntct->isValid())
    q.bindValue(":pohead_vend_cntct_id", _vendCntct->id());
  q.bindValue(":pohead_vend_cntct_honorific", _vendCntct->honorific());
  q.bindValue(":pohead_vend_cntct_first_name", _vendCntct->first());
  q.bindValue(":pohead_vend_cntct_middle", _vendCntct->middle());
  q.bindValue(":pohead_vend_cntct_last_name", _vendCntct->last());
  q.bindValue(":pohead_vend_cntct_suffix", _vendCntct->suffix());
  q.bindValue(":pohead_vend_cntct_phone", _vendCntct->phone());
  q.bindValue(":pohead_vend_cntct_title", _vendCntct->title());
  q.bindValue(":pohead_vend_cntct_fax", _vendCntct->fax());
  q.bindValue(":pohead_vend_cntct_email", _vendCntct->emailAddress());
  q.bindValue(":pohead_vendaddress1", _vendAddr->line1());
  q.bindValue(":pohead_vendaddress2", _vendAddr->line2());
  q.bindValue(":pohead_vendaddress3", _vendAddr->line3());
  q.bindValue(":pohead_vendcity", _vendAddr->city());
  q.bindValue(":pohead_vendstate", _vendAddr->state());
  q.bindValue(":pohead_vendzipcode", _vendAddr->postalCode());
  q.bindValue(":pohead_vendcountry", _vendAddr->country());
  if (_shiptoCntct->isValid())
    q.bindValue(":pohead_shipto_cntct_id", _shiptoCntct->id());
  q.bindValue(":pohead_shipto_cntct_honorific", _shiptoCntct->honorific());
  q.bindValue(":pohead_shipto_cntct_first_name", _shiptoCntct->first());
  q.bindValue(":pohead_shipto_cntct_middle", _shiptoCntct->middle());
  q.bindValue(":pohead_shipto_cntct_last_name", _shiptoCntct->last());
  q.bindValue(":pohead_shipto_cntct_suffix", _shiptoCntct->suffix());
  q.bindValue(":pohead_shipto_cntct_phone", _shiptoCntct->phone());
  q.bindValue(":pohead_shipto_cntct_title", _shiptoCntct->title());
  q.bindValue(":pohead_shipto_cntct_fax", _shiptoCntct->fax());
  q.bindValue(":pohead_shipto_cntct_email", _shiptoCntct->emailAddress());
  if (_shiptoAddr->isValid())
    q.bindValue(":pohead_shiptoddress_id", _shiptoAddr->id());
  q.bindValue(":pohead_shiptoaddress1", _shiptoAddr->line1());
  q.bindValue(":pohead_shiptoaddress2", _shiptoAddr->line2());
  q.bindValue(":pohead_shiptoaddress3", _shiptoAddr->line3());
  q.bindValue(":pohead_shiptocity", _shiptoAddr->city());
  q.bindValue(":pohead_shiptostate", _shiptoAddr->state());
  q.bindValue(":pohead_shiptozipcode", _shiptoAddr->postalCode());
  q.bindValue(":pohead_shiptocountry", _shiptoAddr->country());
  q.bindValue(":pohead_freight", _freight->localValue());
  q.bindValue(":pohead_curr_id", _poCurrency->id());
  if (_status->currentIndex() == 0)
    q.bindValue(":pohead_status", "U");
  else if (_status->currentIndex() == 1)
    q.bindValue(":pohead_status", "O");
  else if (_status->currentIndex() == 2)
    q.bindValue(":pohead_status", "C");
  q.bindValue(":pohead_dropship", QVariant(_dropShip->isChecked()));

  q.exec();
 
  omfgThis->sPurchaseOrdersUpdated(_poheadid, TRUE);

  if (!_pridList.isEmpty())
  {
    q.prepare("SELECT deletePr(:pr_id) AS _result;");
    for(int i = 0; i < _pridList.size(); ++i)
    {
      q.bindValue(":pr_id", _pridList.at(i));
      q.exec();
    }
    omfgThis->sPurchaseRequestsUpdated();
  }

  if (_printPO->isChecked())
  {
    ParameterList params;
    params.append("pohead_id", _poheadid);

    printPurchaseOrder newdlgP(this, "", true);
    newdlgP.set(params);
    newdlgP.exec();
  }

  emit saved(_poheadid);

  if (_mode == cNew && !_captive)
  {
    _purchaseOrderInformation->setCurrentIndex(0);

    _agent->setText(omfgThis->username());
    _terms->setId(-1);
    _vendor->setReadOnly(false);
    _vendor->setId(-1);
    _taxZone->setId(-1);

    _orderNumber->clear();
    _orderDate->clear();
    _shipVia->clear();
    _status->setCurrentIndex(0);
    _fob->clear();
    _notes->clear();
    _tax->clear();
    _freight->clear();
    _total->clear();
    _poitem->clear();
    _poCurrency->setEnabled(true);
    _qecurrency->setEnabled(true);
    _qeitem->removeRows(0, _qeitem->rowCount());
    _vendaddrCode->clear();
    _vendCntct->clear();
    _vendAddr->clear();
    _shiptoCntct->clear();
    _shiptoAddr->clear();

    createHeader();
    _subtotal->clear();
  }
  else
  {
    close();
  }
}

void purchaseOrder::sNew()
{
  if (_mode == cEdit || _mode == cNew)
  {
    q.prepare( "UPDATE pohead "
               "SET pohead_warehous_id=:pohead_warehous_id, pohead_vend_id=:pohead_vend_id,"
               "    pohead_number=:pohead_number, pohead_taxzone_id=:pohead_taxzone_id, "
               "    pohead_curr_id=:pohead_curr_id, "
               "    pohead_orderdate=:pohead_orderdate "
               "WHERE (pohead_id=:pohead_id);" );
    if (_warehouse->isValid())
      q.bindValue(":pohead_warehous_id", _warehouse->id());
    q.bindValue(":pohead_vend_id", _vendor->id());
    q.bindValue(":pohead_number", _orderNumber->text());
    q.bindValue(":pohead_id", _poheadid);
    if (_taxZone->isValid())
      q.bindValue(":pohead_taxzone_id", _taxZone->id());
    q.bindValue(":pohead_curr_id", _poCurrency->id());
    q.bindValue(":pohead_orderdate", _orderDate->date());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("pohead_id", _poheadid);
  params.append("warehous_id", _warehouse->id());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();
    _vendor->setReadOnly(true);
  }
}

void purchaseOrder::sEdit()
{
  saveDetail();
  
  ParameterList params;
  params.append("pohead_id", _poheadid);
  params.append("poitem_id", _poitem->id());

  if (_mode == cEdit || _mode == cNew)
    params.append("mode", "edit");
  else if (_mode == cView)
    params.append("mode", "view");

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void purchaseOrder::sDelete()
{
  if (_deleteMode == cDelete)
  {
    if (QMessageBox::question(this, tr("Delete Purchase Order Item?"),
                                    tr("<p>Are you sure you want to delete this "
                                       "Purchase Order Line Item?"),
           QMessageBox::Yes,
            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    q.prepare( "DELETE FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    q.bindValue(":poitem_id", _poitem->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    if (QMessageBox::question(this, tr("Close Purchase Order Item?"),
                                    tr("<p>Are you sure you want to close this "
                                       "Purchase Order Line Item?"),
           QMessageBox::Yes,
            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    q.prepare( "UPDATE poitem SET poitem_status='C' "
               "WHERE (poitem_id=:poitem_id);" );
    q.bindValue(":poitem_id", _poitem->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillList();
}

void purchaseOrder::sVendaddrList()
{
  ParameterList params;
  params.append("vend_id", _vendor->id());

  vendorAddressList newdlg(this, "", TRUE);
  newdlg.set(params);

  int vendaddrid;
  if ((vendaddrid = newdlg.exec()) != XDialog::Rejected)
  {
    if (vendaddrid != -1)
    {
      q.prepare( "SELECT vendaddr_code, vendaddr_name,"
                 "       vendaddr_address1, vendaddr_address2, vendaddr_address3, "
                 "       vendaddr_city, vendaddr_state, vendaddr_zipcode, vendaddr_country "
                 "FROM vendaddr "
                 "WHERE (vendaddr_id=:vendaddr_id);" );
      q.bindValue(":vendaddr_id", vendaddrid);
      q.exec();
      if (q.first())
      {
        _vendaddrid = vendaddrid;
                disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        _vendaddrCode->setText(q.value("vendaddr_code"));
        _vendAddr->setLine1(q.value("vendaddr_address1").toString());
        _vendAddr->setLine2(q.value("vendaddr_address2").toString());
        _vendAddr->setLine3(q.value("vendaddr_address3").toString());
        _vendAddr->setCity(q.value("vendaddr_city").toString());
        _vendAddr->setState(q.value("vendaddr_state").toString());
        _vendAddr->setPostalCode(q.value("vendaddr_zipcode").toString());
        _vendAddr->setCountry(q.value("vendaddr_country").toString());
                connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
      }
    }
    else
    {
      q.prepare( "SELECT vend_name, "
                 "       vend_address1, vend_address2, vend_address3, "
                 "       vend_city, vend_state, vend_zip, vend_country "
                 "FROM vend "
                 "WHERE (vend_id=:vend_id);" );
      q.bindValue(":vend_id", _vendor->id());
      q.exec();
      if (q.first())
      {
        _vendaddrid = -1;
                disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        _vendaddrCode->setText(tr("Main"));
        _vendAddr->setLine1(q.value("vend_address1").toString());
        _vendAddr->setLine2(q.value("vend_address2").toString());
        _vendAddr->setLine3(q.value("vend_address3").toString());
        _vendAddr->setCity(q.value("vend_city").toString());
        _vendAddr->setState(q.value("vend_state").toString());
        _vendAddr->setPostalCode(q.value("vend_zip").toString());
        _vendAddr->setCountry(q.value("vend_country").toString());
                connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
      }
    }
  }
}

void purchaseOrder::sHandleDeleteButton()
{
  if ( (_mode == cNew) || (_mode == cEdit))
  {
    QTreeWidgetItem *selected = _poitem->currentItem();

    // _poitem->currentItem()->text(1) == values have to match sFillList()
    if (selected == 0)
      _delete->setEnabled(FALSE);
    else if (_poitem->currentItem()->text(1) == tr("Unposted"))
    {
      _deleteMode = cDelete;
      _delete->setEnabled(TRUE);
      _delete->setText(tr("&Delete"));
    }
    //else if (_poitem->currentItem()->text(1) == tr("Open"))
    else
    {
      _deleteMode = cClose;
      _delete->setEnabled(TRUE);
      _delete->setText(tr("C&lose"));
    }
  }
}
 
void purchaseOrder::sHandleVendor(int pVendid)
{
  if ( (pVendid != -1) && (_mode == cNew) )
  {
    q.prepare( "UPDATE pohead "
               "SET pohead_warehous_id=:pohead_warehous_id, "
               " pohead_vend_id=:pohead_vend_id, pohead_curr_id=:pohead_curr_id "
               "WHERE (pohead_id=:pohead_id);" );
    if (_warehouse->isValid())
      q.bindValue(":pohead_warehous_id", _warehouse->id());
    q.bindValue(":pohead_vend_id", pVendid);
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":pohead_curr_id", _poCurrency->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().text(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery vq;
    vq.prepare("SELECT addr.*, cntct.*, vend_terms_id, vend_curr_id,"
               "       vend_fobsource, vend_fob, vend_shipvia,"
               "       vend_name,"
               "       COALESCE(vend_addr_id, -1) AS vendaddrid,"
               "       COALESCE(vend_taxzone_id, -1) AS vendtaxzoneid,"
               "       crmacct_id"
               "  FROM vendinfo JOIN addr ON (vend_addr_id=addr_id)"
               "  JOIN crmacct ON (vend_id=crmacct_vend_id)"
               "  LEFT OUTER JOIN cntct ON (vend_cntct1_id=cntct_id) "
               "WHERE (vend_id=:vend_id) "
               "LIMIT 1;" );
    vq.bindValue(":vend_id", pVendid);
    vq.exec();
    if (vq.first())
    {
      _taxZone->setId(vq.value("vendtaxzoneid").toInt());
      _poCurrency->setId(vq.value("vend_curr_id").toInt());

      if (_terms->id() == -1)
        _terms->setId(vq.value("vend_terms_id").toInt());

      if (_shipVia->currentText().length() == 0)
        _shipVia->setText(vq.value("vend_shipvia"));

      if (vq.value("vend_fobsource").toString() == "V")
      {
        _useWarehouseFOB = FALSE;
        _fob->setText(vq.value("vend_fob"));
      }
      else
      {
        _useWarehouseFOB = TRUE;
        _fob->setText(tr("Destination"));
      }

      if (vq.value("cntct_id").toInt())
      {
        _vendCntct->setId(vq.value("cntct_id").toInt());
        _vendCntct->setHonorific(vq.value("cntct_honorific").toString());
        _vendCntct->setFirst(vq.value("cntct_first_name").toString());
        _vendCntct->setMiddle(vq.value("cntct_middle").toString());
        _vendCntct->setLast(vq.value("cntct_last_name").toString());
        _vendCntct->setSuffix(vq.value("cntct_suffix").toString());
        _vendCntct->setPhone(vq.value("cntct_phone").toString());
        _vendCntct->setTitle(vq.value("cntct_title").toString());
        _vendCntct->setFax(vq.value("cntct_fax").toString());
        _vendCntct->setEmailAddress(vq.value("cntct_email").toString());
      }

      if (vq.value("addr_id").toInt())
      {
        _vendaddrid = -1;
        disconnect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
        _vendaddrCode->setText(tr("Main"));
        _vendAddr->setId(vq.value("addr_id").toInt());
        _vendAddr->setLine1(vq.value("addr_line1").toString());
        _vendAddr->setLine2(vq.value("addr_line2").toString());
        _vendAddr->setLine3(vq.value("addr_line3").toString());
        _vendAddr->setCity(vq.value("addr_city").toString());
        _vendAddr->setState(vq.value("addr_state").toString());
        _vendAddr->setPostalCode(vq.value("addr_postalcode").toString());
        _vendAddr->setCountry(vq.value("addr_country").toString());
        connect(_vendAddr, SIGNAL(changed()), _vendaddrCode, SLOT(clear()));
      }

      if (vq.value("crmacct_id").toInt())
        _vendCntct->setSearchAcct(vq.value("crmacct_id").toInt());
    }
    else if (vq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, vq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }
}

void purchaseOrder::sFillList()
{
  MetaSQLQuery mql = mqlLoad("poItems", "list");

  ParameterList params;
  params.append("pohead_id", _poheadid);
  params.append("closed", tr("Closed"));
  params.append("unposted", tr("Unposted"));
  params.append("partial", tr("Partial"));
  params.append("received", tr("Received"));
  params.append("open", tr("Open"));
  params.append("so", tr("SO"));
  params.append("wo", tr("WO"));

  q = mql.toQuery(params);
  _poitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sCalculateTax();
  sCalculateTotals();

  _poCurrency->setEnabled(_poitem->topLevelItemCount() == 0);
  _qecurrency->setEnabled(_poitem->topLevelItemCount() == 0);

  _qeitem->select();
}

void purchaseOrder::sCalculateTotals()
{
  q.prepare( "SELECT SUM(poitem_qty_ordered * poitem_unitprice) AS total,"
             "       SUM(poitem_qty_ordered * poitem_unitprice) AS f_total,"
             "       SUM(poitem_freight) AS freightsub "
             "FROM poitem "
             "WHERE (poitem_pohead_id=:pohead_id);" );
  q.bindValue(":pohead_id", _poheadid);
  q.exec();
  if (q.first())
  {
    _subtotal->setLocalValue(q.value("f_total").toDouble());
    _totalFreight->setLocalValue(q.value("freightsub").toDouble() + _freight->localValue());
    _total->setLocalValue(q.value("total").toDouble() + _tax->localValue() + q.value("freightsub").toDouble() + _freight->localValue());
  }
}

void purchaseOrder::sSetUserOrderNumber()
{
  _userOrderNumber = TRUE;
}
    
void purchaseOrder::sHandleOrderNumber()
{
  if (_orderNumber->text().length() == 0)
  {
    if ( (_metrics->value("PONumberGeneration") == "A") ||
         (_metrics->value("PONumberGeneration") == "O") )
      populateOrderNumber();
    else
      return;
  }
  else if (_userOrderNumber)
  {
    q.prepare( "SELECT pohead_id "
               "FROM pohead "
               "WHERE (pohead_number=:pohead_number);" );
    q.bindValue(":pohead_number", _orderNumber->text());
    q.exec();
    if (q.first())
    {
      int poheadid = q.value("pohead_id").toInt();
      if(poheadid == _poheadid)
        return;

      if(cNew != _mode || !_pridList.isEmpty())
      {
        QMessageBox::warning( this, tr("Enter Purchase Order #"),
          tr("The Purchase Order number you entered is already in use. Please enter a different number.") );
        _orderNumber->clear();
        _orderNumber->setFocus();
        return;        
      }

      q.prepare( "DELETE FROM pohead "
                 "WHERE (pohead_id=:pohead_id);"
                 "DELETE FROM poitem "
                 "WHERE (poitem_pohead_id=:pohead_id);"
                 "SELECT releasePoNumber(:orderNumber);" );
      q.bindValue(":pohead_id", _poheadid);
      q.bindValue(":orderNumber", _orderNumber->text().toInt());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

      _mode = cEdit;
      setPoheadid(poheadid);
      populate();
      _orderNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _vendor->setReadOnly(TRUE);
    }
  }
  if(_poheadid != -1)
  {
    q.prepare("UPDATE pohead"
              "   SET pohead_number=:pohead_number"
              " WHERE (pohead_id=:pohead_id);");
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":pohead_number", _orderNumber->text());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void purchaseOrder::populateOrderNumber()
{
  XSqlQuery on;
  on.exec("SELECT fetchPoNumber() AS ponumber;");
  if (on.first())
    _orderNumber->setText(on.value("ponumber"));

  if (_metrics->value("PONumberGeneration") == "A")
    _orderNumber->setEnabled(FALSE);
}

void purchaseOrder::closeEvent(QCloseEvent *pEvent)
{
  if ((_mode == cNew) &&
      (_poheadid != -1) &&
      !_captive)
  {
    if (_poitem->topLevelItemCount() > 0 &&
        QMessageBox::question(this, tr("Delete Purchase Order?"),
                              tr("<p>Are you sure you want to delete this "
                                 "Purchase Order and all of its associated "
                                 "Line Items?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      pEvent->ignore();
      return;
    }

    q.prepare( "DELETE FROM pohead "
               "WHERE (pohead_id=:pohead_id);"
               "DELETE FROM poitem "
               "WHERE (poitem_pohead_id=:pohead_id);"
               "SELECT releasePoNumber(:orderNumber);" );
    q.bindValue(":pohead_id", _poheadid);
    q.bindValue(":orderNumber", _orderNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (_mode == cEdit && _poitem->topLevelItemCount() == 0)
  {
    if (QMessageBox::question(this, tr("Delete Purchase Order?"),
                              tr("<p>This Purchase Order does not have any line items.  "
                                  "Are you sure you want to delete this Purchase Order?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      pEvent->ignore();
      return;
    }

    q.prepare( "DELETE FROM pohead "
               "WHERE (pohead_id=:pohead_id);" );
    q.bindValue(":pohead_id", _poheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  // TODO: if sQeSave == false then find a way to return control to the user
  if (_qeitem->isDirty())
  {
    if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
                    tr("Do you want to save your Quick Entry changes?"),
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      sQESave();
  }

  XWidget::closeEvent(pEvent);
}

bool purchaseOrder::sQESave()
{
  _qesave->setFocus();
  if (! _qeitem->submitAll())
  {
    if (! _qeitem->lastError().databaseText().isEmpty())
      systemError(this, _qeitem->lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  sFillList();
  return true;
}

void purchaseOrder::sQEDelete()
{
  if (! _qeitem->removeRow(_qeitemView->currentIndex().row()))
  {
    systemError(this, tr("Removing row from view failed"), __FILE__, __LINE__);
    return;
  }
}

void purchaseOrder::sCurrencyChanged()
{
  // The user can only set the QE currency from the QE tab
  if (_purchaseOrderInformation->currentWidget()->isAncestorOf(_qecurrency) &&
      _poCurrency->id() != _qecurrency->id())
    _poCurrency->setId(_qecurrency->id());
  else
    _qecurrency->setId(_poCurrency->id());

  static_cast<PoitemTableModel*>(_qeitemView->model())->setCurrId(_poCurrency->id());
}

void purchaseOrder::sTabChanged(int pIndex)
{
  if (pIndex != _cachedTabIndex &&
      _cachedTabIndex == _purchaseOrderInformation->indexOf(_quickEntryTab) &&
      _qeitem->isDirty())
  {
    if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
                    tr("Do you want to save your Quick Entry changes?"),
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      if (! sQESave())
      {
        _purchaseOrderInformation->setCurrentIndex(_cachedTabIndex);
        return;
      }
  }
  _cachedTabIndex = pIndex;
}

void purchaseOrder::sHandleOrderDate()
{
  static_cast<PoitemTableModel*>(_qeitemView->model())->setTransDate(_orderDate->date());
}

void purchaseOrder::sTaxZoneChanged()
{
 if (_poheadid == -1 )
  {
    XSqlQuery taxq;
    taxq.prepare("UPDATE pohead SET "
      "  pohead_taxzone_id=:taxzone_id "
      "WHERE (pohead_id=:pohead_id) ");
    if (_taxZone->isValid())
      taxq.bindValue(":taxzone_id", _taxZone->id());
    taxq.bindValue(":pohead_id", _poheadid);
    taxq.bindValue(":freight", _freight->localValue());
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sCalculateTax();
  }
}

void purchaseOrder::sCalculateTax()
{  
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('PO', :pohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
                "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":pohead_id", _poheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }              
}

void purchaseOrder::sTaxDetail()
{
  saveDetail();

  ParameterList params;
  params.append("order_id", _poheadid);
  params.append("order_type", "PO");
  // mode => view since there are no fields to hold modified tax data
  if (_mode == cView)
    params.append("mode", "view");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    populate();
}

void purchaseOrder::saveDetail()
{
  XSqlQuery taxq;
  if (_mode != cView)
  {
    taxq.prepare("UPDATE pohead SET pohead_taxzone_id = :taxzone,"
                 " pohead_orderdate = :pohead_orderdate,"
                                 " pohead_curr_id = :pohead_curr_id,"
                                 " pohead_freight = :pohead_freight "
                         "WHERE (pohead_id = :pohead_id);");
    if (_taxZone->isValid())
      taxq.bindValue(":taxzone",        _taxZone->id());
    taxq.bindValue(":pohead_id",        _poheadid);
    taxq.bindValue(":pohead_orderdate", _orderDate->date());
        taxq.bindValue(":pohead_curr_id", _poCurrency->id());
        taxq.bindValue(":pohead_freight", _freight->localValue());
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void purchaseOrder::sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  QAction *menuItem;

  if (pSelected->text(_poitem->column("demand_type")) == tr("SO"))
  {
    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewSo()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));

    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEditSo()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  }

  else if (pSelected->text(_poitem->column("demand_type")) == "WO")
  {
    menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sViewWo()));
    menuItem->setEnabled(_privileges->check("ViewWorkOrders")); 

    menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sEditWo()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
  }
}

void purchaseOrder::sViewSo()
{
  XSqlQuery fetchso;
  fetchso.prepare( "SELECT pohead_cohead_id "
                   "FROM pohead "
                   "WHERE pohead_id = :pohead_id " );
  fetchso.bindValue(":pohead_id", _poheadid);
  fetchso.exec();
  if (fetchso.first())
    salesOrder::viewSalesOrder(fetchso.value("pohead_cohead_id").toInt());
}

void purchaseOrder::sEditSo()
{
  XSqlQuery fetchso;
  fetchso.prepare( "SELECT pohead_cohead_id "
                   "FROM pohead "
                   "WHERE pohead_id = :pohead_id " );
  fetchso.bindValue(":pohead_id", _poheadid);
  fetchso.exec();
  if (fetchso.first())
    salesOrder::editSalesOrder(fetchso.value("pohead_cohead_id").toInt(), TRUE);
}

void purchaseOrder::sViewWo()
{
  XSqlQuery fetchwo;
  fetchwo.prepare( "SELECT wo_id "
                   "FROM pohead JOIN poitem ON (poitem_pohead_id=pohead_id )"
                   "     JOIN wo ON (poitem_wohead_id=wo_id) "
                   "WHERE (pohead_id=:pohead_id);" );
  fetchwo.bindValue(":pohead_id", _poheadid);
  fetchwo.exec();
  if (fetchwo.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", (fetchwo.value("wo_id").toInt()));

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void purchaseOrder::sEditWo()
{
  XSqlQuery fetchwo;
  fetchwo.prepare( "SELECT wo_id "
                   "FROM pohead JOIN poitem ON (poitem_pohead_id=pohead_id )"
                   "     JOIN wo ON (poitem_wohead_id=wo_id) "
                   "WHERE (pohead_id=:pohead_id);" );
  fetchwo.bindValue(":pohead_id", _poheadid);
  fetchwo.exec();
  if (fetchwo.first())
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("wo_id", (fetchwo.value("wo_id").toInt()));

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void purchaseOrder::sHandleShipTo()
{
  q.prepare( "SELECT cntct.*, addr.* "
             "FROM whsinfo LEFT OUTER JOIN cntct ON (warehous_cntct_id=cntct_id)"
             "     LEFT OUTER JOIN addr ON (warehous_addr_id=addr_id) "
             "WHERE (warehous_id=:warehous_id);" );
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    _shiptoCntct->setId(q.value("cntct_id").toInt());
    _shiptoCntct->setHonorific(q.value("cntct_honorific").toString());
    _shiptoCntct->setFirst(q.value("cntct_first_name").toString());
    _shiptoCntct->setMiddle(q.value("cntct_middle").toString());
    _shiptoCntct->setLast(q.value("cntct_last_name").toString());
    _shiptoCntct->setSuffix(q.value("cntct_suffix").toString());
    _shiptoCntct->setPhone(q.value("cntct_phone").toString());
    _shiptoCntct->setTitle(q.value("cntct_title").toString());
    _shiptoCntct->setFax(q.value("cntct_fax").toString());
    _shiptoCntct->setEmailAddress(q.value("cntct_email").toString());

        _shiptoAddr->setId(q.value("addr_id").toInt());
    _shiptoAddr->setLine1(q.value("addr_line1").toString());
    _shiptoAddr->setLine2(q.value("addr_line2").toString());
    _shiptoAddr->setLine3(q.value("addr_line3").toString());
    _shiptoAddr->setCity(q.value("addr_city").toString());
    _shiptoAddr->setState(q.value("addr_state").toString());
    _shiptoAddr->setPostalCode(q.value("addr_postalcode").toString());
    _shiptoAddr->setCountry(q.value("addr_country").toString());
  }
}
