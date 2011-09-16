/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transferOrder.h"

#include <stdlib.h>

#include <QAction>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "transferOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

const char *_statusTypes[] = { "U", "O", "C" };

#define cClosed       0x01
#define cActiveOpen   0x02
#define cInactiveOpen 0x04
#define cCanceled     0x08
#define cUnreleased   0x16

transferOrder::transferOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _qeitem = new ToitemTableModel(this);
  _qeitemView->setModel(_qeitem);

  connect(_action,	    SIGNAL(clicked()), this, SLOT(sAction()));
  connect(_clear,	    SIGNAL(pressed()), this, SLOT(sClear()));
  connect(_delete,	    SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_dstWhs,         SIGNAL(newID(int)), this, SLOT(sHandleDstWhs(int)));
  connect(_edit,	    SIGNAL(clicked()), this, SLOT(sEdit()));
//  connect(_freight,    SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_freightCurrency, SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));
  connect(_issueLineBalance, SIGNAL(clicked()), this, SLOT(sIssueLineBalance()));
  connect(_issueStock,	     SIGNAL(clicked()), this, SLOT(sIssueStock()));
  connect(_new,		     SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_orderDate,	SIGNAL(newDate(QDate)), _qeitem, SLOT(setTransDate(QDate)));
  connect(_orderNumber,    SIGNAL(lostFocus()), this, SLOT(sHandleOrderNumber()));
  connect(_orderNumber,	SIGNAL(textChanged(const QString&)), this, SLOT(sSetUserEnteredOrderNumber()));
  connect(_packDate,	SIGNAL(newDate(QDate)), _qeitem, SLOT(setShipDate(QDate)));
  connect(_qecurrency,	   SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));
  connect(_qedelete,	    SIGNAL(clicked()), this, SLOT(sQEDelete()));
  connect(_qesave,	    SIGNAL(clicked()), this, SLOT(sQESave()));
  connect(_save,	    SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_saveAndAdd,	    SIGNAL(clicked()), this, SLOT(sSaveAndAdd()));
  connect(_shipDate,   SIGNAL(newDate(QDate)), _qeitem, SLOT(setShipDate(QDate)));
  connect(_showCanceled,SIGNAL(toggled(bool)), this, SLOT(sFillItemList()));
  connect(_srcWhs,         SIGNAL(newID(int)), this, SLOT(sHandleSrcWhs(int)));
  connect(_tabs,  SIGNAL(currentChanged(int)), this, SLOT(sTabChanged(int)));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxzone,        SIGNAL(newID(int)), this, SLOT(sCalculateTax()));
  connect(_toitem, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_toitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_trnsWhs,  SIGNAL(newID(int)), this, SLOT(sHandleTrnsWhs(int)));
  connect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sHandleTransferOrderEvent(int)));

  _toitem->addColumn(tr("#"),           _seqColumn,     Qt::AlignCenter, true,  "toitem_linenumber" );
  _toitem->addColumn(tr("Item"),        _itemColumn,    Qt::AlignLeft,   true,  "item_number"   );
  _toitem->addColumn(tr("Description"), -1,             Qt::AlignLeft,   true,  "description"   );
  _toitem->addColumn(tr("Status"),      _statusColumn,  Qt::AlignCenter, true,  "toitem_status" );
  _toitem->addColumn(tr("Sched. Date"), _dateColumn,    Qt::AlignCenter, true,  "toitem_schedshipdate" );
  _toitem->addColumn(tr("Ordered"),     _qtyColumn,     Qt::AlignRight,  true,  "toitem_qty_ordered"  );
  _toitem->addColumn(tr("At Shipping"), _qtyColumn,     Qt::AlignRight,  true,  "atshipping"  );
  _toitem->addColumn(tr("Shipped"),     _qtyColumn,     Qt::AlignRight,  true,  "toitem_qty_shipped"  );
  _toitem->addColumn(tr("Balance"),     _qtyColumn,     Qt::AlignRight,  true,  "balance"  );
  _toitem->addColumn(tr("Received"),    _qtyColumn,     Qt::AlignRight,  true,  "toitem_qty_received" );

  _orderNumber->setValidator(omfgThis->orderVal());

  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _mode = cView;

  setToheadid(-1);

  _captive		= false;
  _freighttaxid		= -1;
  _orderNumberGen	= 0;
  _saved		= false;
  _taxzoneidCache	= -1;
  _whstaxzoneid		= -1;
  _locked       = false;
  _srcWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _trnsWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());
  _dstWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  getWhsInfo(_trnsWhs->id(), _trnsWhs);

  _weight->setValidator(omfgThis->qtyVal());
  
  // TODO: remove when 5695 is fixed
  _freight->hide();
  _freightLit->hide();
  // end 5695
}

void transferOrder::setToheadid(const int pId)
{
  _toheadid = pId;
  _qeitem->setHeadId(pId);
  _comments->setId(_toheadid);
       
  if(_mode == cEdit && !_locked)
  {
     XSqlQuery to;
     to.prepare("SELECT tryLock(oid::integer, :tohead_id) AS locked "
                "FROM pg_class "
                "WHERE relname='tohead';");
     to.bindValue(":tohead_id", _toheadid);
     to.exec();
     if(to.first())
     {
       if(to.value("locked").toBool() != true)
       {
          QMessageBox::critical( this, tr("Record Currently Being Edited"),
               tr("<p>The record you are trying to edit is currently being edited "
               "by another user. Continue in View Mode.") );
          setViewMode();
       }
       else
         _locked=true;
     }
     else
     {
       QMessageBox::critical( this, tr("Cannot Lock Record for Editing"),
             tr("<p>There was an unexpected error while trying to lock the record "
             "for editing. Please report this to your administator.") );
       setViewMode();
     }
  }
}

transferOrder::~transferOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void transferOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse transferOrder::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      itemid;
  int      srcwarehousid = -1;
  int      destwarehousid = -1;
  double   qty;
  QDate    dueDate;
  
  setToheadid(-1);
  int      _planordid = -1;

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("transferOrder new");
      _mode = cNew;
      if ( (_metrics->value("TONumberGeneration") == "A") ||
           (_metrics->value("TONumberGeneration") == "O")   )
      {
        if (! insertPlaceholder())
          return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      if(_metrics->boolean("AlwaysShowSaveAndAdd"))
        _saveAndAdd->setEnabled(true);
      else
        _saveAndAdd->hide();
    }
    else if (param.toString() == "view")
      setViewMode();
    else if (param.toString() == "releaseTO")
    {
      _mode = cNew;
      q.prepare( "SELECT planord.*,"
                 "       srcsite.itemsite_item_id AS itemid,"
                 "       srcsite.itemsite_warehous_id AS srcwarehousid,"
                 "       destsite.itemsite_warehous_id AS destwarehousid "
                 "FROM planord JOIN itemsite srcsite  ON (srcsite.itemsite_id=planord_supply_itemsite_id) "
                 "             JOIN itemsite destsite ON (destsite.itemsite_id=planord_itemsite_id) "
                 "WHERE (planord_id=:planord_id);" );
      q.bindValue(":planord_id", _planordid);
      q.exec();
      if (q.first())
      {
        itemid = q.value("itemid").toInt();
        qty = q.value("planord_qty").toDouble();
        dueDate = q.value("planord_duedate").toDate();
        srcwarehousid = q.value("srcwarehousid").toInt();
        destwarehousid = q.value("destwarehousid").toInt();
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      connect(_toitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_toitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_toitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      q.prepare( "SELECT tohead_id "
                 "FROM tohead "
                 "WHERE ( (tohead_status='U')"
                 "  AND   (tohead_src_warehous_id=:tohead_src_warehous_id) "
                 "  AND   (tohead_dest_warehous_id=:tohead_dest_warehous_id) ) "
                 "ORDER BY tohead_number "
                 "LIMIT 1;" );
      q.bindValue(":tohead_src_warehous_id", srcwarehousid);
      q.bindValue(":tohead_dest_warehous_id", destwarehousid);
      q.exec();
      if (q.first())
      {
//  Transfer order found
        if(QMessageBox::question( this, tr("Unreleased Transfer Order Exists"),
                                        tr("An Unreleased Transfer Order\n"
                                           "already exists for this\n"
                                           "Source/Destination Warehouse.\n"
                                           "Would you like to use this Transfer Order?\n"
                                           "Click Yes to use the existing Transfer Order\n"
                                           "otherwise a new one will be created."),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
//  Use an existing order
          _mode = cEdit;

          setToheadid(q.value("tohead_id").toInt());
          _orderNumber->setEnabled(FALSE);
          _orderDate->setEnabled(FALSE);
          populate();
        }
        else
        {
//  Do not use an existing order, create new
          _srcWhs->setId(srcwarehousid);
          _dstWhs->setId(destwarehousid);
          insertPlaceholder();
        }
      }
      else
      {
//  Transfer order not found, create new
        _srcWhs->setId(srcwarehousid);
        _dstWhs->setId(destwarehousid);
        insertPlaceholder();
      }

//  Start to create the new toitem
      ParameterList newItemParams;
      newItemParams.append("mode", "new");
      newItemParams.append("tohead_id", _toheadid);
      newItemParams.append("srcwarehouse_id", _srcWhs->id());
      newItemParams.append("orderNumber",	_orderNumber->text());
      newItemParams.append("orderDate",	_orderDate->date());
      newItemParams.append("taxzone_id",	_taxzone->id());
      newItemParams.append("curr_id",	_freightCurrency->id());
      newItemParams.append("item_id", itemid);
      newItemParams.append("captive", TRUE);

      if (qty > 0.0)
        newItemParams.append("qty", qty);

      if (!dueDate.isNull())
        newItemParams.append("dueDate", dueDate);

      transferOrderItem toItem(this, "", TRUE);
      toItem.set(newItemParams);
// TODO
//      if (toItem.exec() != XDialog::Rejected)
      if (toItem.exec() == XDialog::Rejected)
      {
        q.prepare("SELECT deletePlannedOrder(:planord_id, FALSE) AS _result;");
        q.bindValue(":planord_id", _planordid);
        q.exec();
// TODO
//        omfgThis->sPlannedOrdersUpdated();
        if(_mode == cEdit)
        {
          // check for another open window
          QWidgetList list = omfgThis->windowList();
          for(int i = 0; i < list.size(); i++)
          {
            QWidget * w = list.at(i);
            if(QString::compare(w->metaObject()->className(), "transferOrder") &&
               w != this)
            {
              transferOrder *other = (transferOrder*)w;
              if(_toheadid == other->_toheadid)
              {
                other->sFillItemList();
                other->setFocus();
                return UndefinedError;
              }
            }
          }
        }
        sFillItemList();
      }
      else
        _planordid = -1;
    }
  }

  param = pParams.value("src_warehous_id", &valid);
  if (valid)
    _srcWhs->setId(param.toInt());

  param = pParams.value("dst_warehous_id", &valid);
  if (valid)
    _dstWhs->setId(param.toInt());

  if (cNew == _mode)
  {
    if (_orderNumber->text().isEmpty())
      _orderNumber->setFocus();
    else
      _packDate->setFocus();

    _status->setCurrentIndex(0);

    _captive = FALSE;
    _edit->setEnabled(FALSE);
    _action->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _close->setText("&Cancel");
  }
  else if (cEdit == _mode)
  {
    _captive = TRUE;
    _orderNumber->setEnabled(FALSE);

    _new->setFocus();
  }
  else if (cView == _mode)
  {
    _project->setReadOnly(true);

    _close->setFocus();
  }

  if( !_metrics->boolean("EnableTOShipping"))
  {
    _requireInventory->hide();
    _issueStock->hide();
    _issueLineBalance->hide();
    _toitem->hideColumn(7);
    _toitem->hideColumn(8);
  }
  else
    _toitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    setToheadid(param.toInt());
    if(cEdit == _mode)
      setObjectName(QString("transferOrder edit %1").arg(_toheadid));
    else if(cView == _mode)
      setObjectName(QString("transferOrder view %1").arg(_toheadid));
    populate();
  }

  if ( pParams.inList("enableSaveAndAdd") &&
       ((_mode == cNew) || (_mode == cEdit)) )
  {
    _saveAndAdd->show();
    _saveAndAdd->setEnabled(true);
  }

  if (cNew == _mode || cEdit == _mode)
  {
    _orderDate->setEnabled(_privileges->check("OverrideTODate"));
    _packDate->setEnabled(_privileges->check("AlterPackDate"));
  }
  else
  {
    _orderDate->setEnabled(FALSE);
    _packDate->setEnabled(FALSE);
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  // TODO: Why don't the constructor or setId earlier in set() handle this?
  getWhsInfo(_srcWhs->id(), _srcWhs);
  getWhsInfo(_dstWhs->id(), _dstWhs);

  return NoError;
}

bool transferOrder::insertPlaceholder()
{
  if(_trnsWhs->id() == -1)
  {
    QMessageBox::critical(this, tr("No Transit Site"),
      tr("There are no transit sites defined in the system."
         " You must define at least one transit site to use Transfer Orders.") );
    return false;
  }

  q.exec("SELECT NEXTVAL('tohead_tohead_id_seq') AS head_id;");
  if (q.first())
  {
    setToheadid(q.value("head_id").toInt());
    _orderDate->setDate(omfgThis->dbDate(), true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return UndefinedError;
  }

  _ignoreSignals = TRUE;
  populateOrderNumber();
  _ignoreSignals = FALSE;

  q.prepare("INSERT INTO tohead ("
	    "          tohead_id, tohead_number, tohead_src_warehous_id,"
	    "          tohead_trns_warehous_id, tohead_dest_warehous_id,"
	    "          tohead_status, tohead_shipform_id"
	    ") VALUES ("
	    "          :tohead_id, :tohead_number, :tohead_src_warehous_id,"
	    "          :tohead_trns_warehous_id, :tohead_dest_warehous_id,"
	    "          :tohead_status, :tohead_shipform_id);");

  q.bindValue(":tohead_id", _toheadid);
  if (_orderNumber->text().isEmpty())
    q.bindValue(":tohead_number", QString::number(_toheadid * -1));
  else
    q.bindValue(":tohead_number",	  _orderNumber->text());
  q.bindValue(":tohead_src_warehous_id",  _srcWhs->id());
  q.bindValue(":tohead_trns_warehous_id", _trnsWhs->id());
  q.bindValue(":tohead_dest_warehous_id", _dstWhs->id());

  q.bindValue(":tohead_status", "U");
  
  if (_shippingForm->isValid())
    q.bindValue(":tohead_shipform_id",	  _shippingForm->id());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void transferOrder::sSaveAndAdd()
{
  if (save(false))
  {
    q.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
    q.bindValue(":tohead_id", _toheadid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	      systemError(this,
		    storedProcErrorLookup("addToPackingListBatch", result),
		    __FILE__, __LINE__);
	      return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sReleaseTohead();
    if (_captive)
      close();
    else
      clear();
  }
}

void transferOrder::sSave()
{
  if (save(false))
  {
    sReleaseTohead();
    if (_captive)
      close();
    else
      clear();
  }
}

bool transferOrder::save(bool partial)
{
  struct {
    bool	condition;
    QString	msg;
    QWidget	*widget;
  } error[] = {
    { _srcWhs->id() == -1,
      tr("You must select a Source Site for this "
	 "Transfer Order before you may save it."),
    _srcWhs
    },
    { _trnsWhs->id() == -1,	// but see fr 5581
      tr("You must select a Transit Site for "
	 "this Transfer Order before you may save it."),
      _dstWhs
    },
    { _dstWhs->id() == -1,
      tr("You must select a Destination Site for "
	 "this Transfer Order before you may save it."),
      _dstWhs
    },
    { _srcWhs->id() == _dstWhs->id(),
      tr("The Source and Destination Sites must be different."),
      _dstWhs
    },
    { (!partial && _toitem->topLevelItemCount() == 0),
      tr("You must create at least one Line Item for "
      "this Transfer Order before you may save it."),
      _new
    },
    { _orderNumber->text().toInt() == 0,
      tr("You must enter a valid T/O # for this Transfer"
	 "Order before you may save it."),
      _orderNumber
    },
    { true, "", 0 }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Transfer Order"),
			  QString("<p>") + error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return false;
  }

  if (_qeitem->isDirty() && ! sQESave())
    return false;

  // place holder for automatic project creation
  // if(! partial && _metrics->boolean("AutoCreateProjectsForOrders") &&
  //    (_project->id() == -1) && (cNew == _mode))

  // save contact and address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  // TODO: put in real checking - perhaps the address of the warehouse is wrong
  _ignoreSignals = TRUE;
  _srcAddr->save(AddressCluster::CHANGEONE);
  _dstAddr->save(AddressCluster::CHANGEONE);
  _ignoreSignals = FALSE;

  q.prepare("UPDATE tohead "
	    "SET tohead_number=:number,"
	    "    tohead_status=:status,"
	    "    tohead_orderdate=:orderdate,"
	    "    tohead_src_warehous_id=:src_warehous_id,"
	    "    tohead_srcname=:srcname,"
	    "    tohead_srcaddress1=:srcaddress1,"
	    "    tohead_srcaddress2=:srcaddress2,"
	    "    tohead_srcaddress3=:srcaddress3,"
	    "    tohead_srccity=:srccity,"
	    "    tohead_srcstate=:srcstate,"
	    "    tohead_srcpostalcode=:srcpostalcode,"
	    "    tohead_srccountry=:srccountry,"
	    "    tohead_srccntct_id=:srccntct_id,"
	    "    tohead_srccntct_name=:srccntct_name,"
	    "    tohead_srcphone=:srcphone,"
	    "    tohead_trns_warehous_id=:trns_warehous_id,"
	    "    tohead_trnsname=:trnsname,"
	    "    tohead_dest_warehous_id=:dest_warehous_id,"
	    "    tohead_destname=:destname,"
	    "    tohead_destaddress1=:destaddress1,"
	    "    tohead_destaddress2=:destaddress2,"
	    "    tohead_destaddress3=:destaddress3,"
	    "    tohead_destcity=:destcity,"
	    "    tohead_deststate=:deststate,"
	    "    tohead_destpostalcode=:destpostalcode,"
	    "    tohead_destcountry=:destcountry,"
	    "    tohead_destcntct_id=:destcntct_id,"
	    "    tohead_destcntct_name=:destcntct_name,"
	    "    tohead_destphone=:destphone,"
	    "    tohead_agent_username=:agent_username,"
	    "    tohead_shipvia=:shipvia,"
	    "    tohead_shipform_id=:shipform_id,"
	    "    tohead_taxzone_id=:taxzone_id,"
	    "    tohead_freight=:freight,"
	    "    tohead_freight_curr_id=:freight_curr_id,"
	    "    tohead_shipcomplete=:shipcomplete,"
	    "    tohead_ordercomments=:ordercomments,"
	    "    tohead_shipcomments=:shipcomments,"
	    "    tohead_packdate=:packdate,"
	    "    tohead_prj_id=:prj_id,"
	    "    tohead_lastupdated=CURRENT_TIMESTAMP "
	   "WHERE (tohead_id=:id);" );

  q.bindValue(":id", _toheadid );

  q.bindValue(":number",		_orderNumber->text().toInt());
  q.bindValue(":status",    _statusTypes[_status->currentIndex()]);
  q.bindValue(":orderdate",		_orderDate->date());
  q.bindValue(":src_warehous_id",	_srcWhs->id());
  q.bindValue(":srcname",		_srcWhs->currentText());
  q.bindValue(":srcaddress1",		_srcAddr->line1());
  q.bindValue(":srcaddress2",		_srcAddr->line2());
  q.bindValue(":srcaddress3",		_srcAddr->line3());
  q.bindValue(":srccity",		_srcAddr->city());
  q.bindValue(":srcstate",		_srcAddr->state());
  q.bindValue(":srcpostalcode",		_srcAddr->postalCode());
  q.bindValue(":srccountry",		_srcAddr->country());
  if (_srcContact->id() > 0)
    q.bindValue(":srccntct_id",		_srcContact->id());
  q.bindValue(":srccntct_name",		_srcContact->name());
  q.bindValue(":srcphone",		_srcContact->phone());

  q.bindValue(":trns_warehous_id",	_trnsWhs->id());
  q.bindValue(":trnsname",		_trnsWhs->currentText());

  q.bindValue(":dest_warehous_id",	_dstWhs->id());
  q.bindValue(":destname",		_dstWhs->currentText());
  q.bindValue(":destaddress1",		_dstAddr->line1());
  q.bindValue(":destaddress2",		_dstAddr->line2());
  q.bindValue(":destaddress3",		_dstAddr->line3());
  q.bindValue(":destcity",		_dstAddr->city());
  q.bindValue(":deststate",		_dstAddr->state());
  q.bindValue(":destpostalcode",	_dstAddr->postalCode());
  q.bindValue(":destcountry",		_dstAddr->country());
  if (_dstContact->id() > 0)
    q.bindValue(":destcntct_id",	_dstContact->id());
  q.bindValue(":destcntct_name",	_dstContact->name());
  q.bindValue(":destphone",		_dstContact->phone());

  q.bindValue(":agent_username",	_agent->currentText());
  q.bindValue(":shipvia",		_shipVia->currentText());

  if (_taxzone->isValid())
    q.bindValue(":taxzone_id",		_taxzone->id());

  q.bindValue(":freight",		_freight->localValue());
  q.bindValue(":freight_curr_id",	_freight->id());

  q.bindValue(":shipcomplete",	QVariant(_shipComplete->isChecked()));
  q.bindValue(":ordercomments",		_orderComments->toPlainText());
  q.bindValue(":shipcomments",		_shippingComments->toPlainText());

  if (_packDate->isValid())
    q.bindValue(":packdate", _packDate->date());
  else
    q.bindValue(":packdate", _orderDate->date());

  if (_project->isValid())
    q.bindValue(":prj_id",		_project->id());

  if (_shippingForm->isValid())
    q.bindValue(":shipform_id",		_shippingForm->id());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  // if this is a new to record and we haven't saved already
  // then we need to lock this record.
  if((cNew == _mode) && (!_saved))
  {
    // should I bother to check because no one should have this but us?
    q.prepare("SELECT tryLock(oid::integer, :head_id) AS locked "
              "FROM pg_class "
              "WHERE relname='tohead';");
    q.bindValue(":head_id", _toheadid);
    q.exec();
    if (q.first())
      _locked = q.value("locked").toBool();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  // keep the status of toitems in synch with tohead
  if (_statusTypes[_status->currentIndex()] == QString("U"))
  {
    q.prepare("SELECT unreleaseTransferOrder(:head_id) AS result;");
    q.bindValue(":head_id", _toheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
  else if (_statusTypes[_status->currentIndex()] == QString("O"))
  {
    q.prepare("SELECT releaseTransferOrder(:head_id) AS result;");
    q.bindValue(":head_id", _toheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
  else if (_statusTypes[_status->currentIndex()] == QString("C"))
  {
    q.prepare("SELECT closeTransferOrder(:head_id) AS result;");
    q.bindValue(":head_id", _toheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  _saved = true;
  omfgThis->sTransferOrdersUpdated(_toheadid);

  emit saved(_toheadid);

  return TRUE;
}

void transferOrder::sPopulateMenu(QMenu *pMenu)
{
  if ((_mode == cNew) || (_mode == cEdit))
  {
    int _numSelected = _toitem->selectedItems().size();
    if(_numSelected == 1)
    {
      if (_lineMode == cClosed)
        pMenu->addAction(tr("Open Line..."), this, SLOT(sAction()));
      else if (_lineMode == cActiveOpen)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
      }
      else if (_lineMode == cInactiveOpen)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
        pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
      }
      else if (_lineMode == cUnreleased)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
      }
    }

    if(_metrics->boolean("EnableTOShipping"))
    {
      if(_numSelected == 1)
        pMenu->addSeparator();

      pMenu->addAction(tr("Return Stock"), this, SLOT(sReturnStock()));
      pMenu->addAction(tr("Issue Stock..."), this, SLOT(sIssueStock()));
      pMenu->addAction(tr("Issue Line Balance"), this, SLOT(sIssueLineBalance()));
    }
  }
}
 
void transferOrder::populateOrderNumber()
{
  if (_mode == cNew)
  {
    if ( (_metrics->value("TONumberGeneration") == "A") ||
         (_metrics->value("TONumberGeneration") == "O")   )
    {
      q.exec("SELECT fetchToNumber() AS tonumber;");
      if (q.first())
      {
        _orderNumber->setText(q.value("tonumber"));
        _orderNumberGen = q.value("tonumber").toInt();

        if (_metrics->value("TONumberGeneration") == "A")
          _orderNumber->setEnabled(FALSE);
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    _new->setEnabled(! _orderNumber->text().isEmpty());
  }

  _userEnteredOrderNumber = FALSE;
}

void transferOrder::sSetUserEnteredOrderNumber()
{
  _userEnteredOrderNumber = TRUE;
}
    
void transferOrder::sHandleOrderNumber()
{
  if (_ignoreSignals || !isActiveWindow())
    return;

  if (_orderNumber->text().length() == 0)
  {

    if (_mode == cNew)
    {
      if ( (_metrics->value("TONumberGeneration") == "A") ||
           (_metrics->value("TONumberGeneration") == "O") )
        populateOrderNumber();
      else
      {
        if(!_close->hasFocus())
        {
        QMessageBox::warning( this, tr("Enter T/O #"),
                              tr("<p>You must enter a T/O # for this Transfer "
				 "Order before you may continue." ) );
        _orderNumber->setFocus();
        }
        return;
      }
    }

  }
  else
  {
    XSqlQuery query;
    if ( (_mode == cNew) && (_userEnteredOrderNumber) )
    {
      if ((_metrics->value("TONumberGeneration") != "A") && 
    	  (_metrics->value("TONumberGeneration") != "O"))
        insertPlaceholder();

      query.prepare( "SELECT tohead_id "
                     "FROM tohead "
                     "WHERE (tohead_number=:tohead_number);" );
      query.bindValue(":tohead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
//        _mode = cEdit;
        setToheadid(query.value("tohead_id").toInt());
        populate();
        _orderNumber->setEnabled(FALSE);
      }
      else
      {
        QString orderNumber = _orderNumber->text();
        query.prepare( "SELECT releaseToNumber(:orderNumber) AS result;" );
        query.bindValue(":orderNumber", _orderNumberGen);
        query.exec();
	if (query.first())
	{
	  int result = query.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("releaseToNumber", result),
			__FILE__, __LINE__);
	    return;
	  }
	}
	else if (query.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
        _orderNumber->setText(orderNumber);
        _userEnteredOrderNumber = FALSE;
        _orderNumber->setEnabled(FALSE);
      }
    }

  }

  if (cView != _mode)
    _new->setEnabled(! _orderNumber->text().isEmpty());
  
  sHandleTrnsWhs(_trnsWhs->id());
  sHandleSrcWhs(_srcWhs->id());
}

void transferOrder::sNew()
{
  if( !_saved && ((_mode == cNew) ) )
    if(!save(true))
      return;

  ParameterList params;
  params.append("tohead_id",	_toheadid);
  params.append("srcwarehouse_id", _srcWhs->id());
  params.append("orderNumber",	_orderNumber->text());
  params.append("orderDate",	_orderDate->date());
  params.append("taxzone_id",	_taxzone->id());
  params.append("curr_id",	_freightCurrency->id());

  if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "new");

  transferOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillItemList();
}

void transferOrder::sEdit()
{
  ParameterList params;
  params.append("toitem_id", _toitem->id());

  if (_mode == cView)
    params.append("mode", "view");
  else if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "edit");

  transferOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted &&
      ((_mode == cNew) || (_mode == cEdit)) )
  {
    sFillItemList();
    sCalculateTax();
  }
    
}

void transferOrder::sHandleButtons()
{
  XTreeWidgetItem *selected = 0;
  QList<XTreeWidgetItem*> selectedlist = _toitem->selectedItems();
  int _numSelected = selectedlist.size();
  if (_numSelected > 0)
    selected = (XTreeWidgetItem*)(selectedlist[0]);
  
  if (selected)
  {
    _issueStock->setEnabled(_privileges->check("IssueStockToShipping"));
    _issueLineBalance->setEnabled(_privileges->check("IssueStockToShipping"));

    if(_numSelected == 1)
    {
      _edit->setEnabled(TRUE);
      int lineMode = selected->altId();

      if (lineMode == 1)
      {
        _lineMode = cClosed;

        _action->setText(tr("Open"));
        _action->setEnabled(TRUE);
        _delete->setEnabled(FALSE);
      }
      else if (lineMode == 2)
      {
        _lineMode = cActiveOpen;

        _action->setText(tr("Close"));
        _action->setEnabled(TRUE);
        _delete->setEnabled(FALSE);
      }
      else if (lineMode == 3)
      {
        _lineMode = cInactiveOpen;

        _action->setText(tr("Close"));
        _action->setEnabled(TRUE);
        _delete->setEnabled(TRUE);
      }
      else if (lineMode == 4)
      {
        _lineMode = cCanceled;

        _action->setEnabled(FALSE);
        _delete->setEnabled(FALSE);
      }
      else if (lineMode == 5)
      {
        _lineMode = cUnreleased;

        _action->setText(tr("Close"));
        _action->setEnabled(FALSE);
        _delete->setEnabled(TRUE);
      }
      else
      {
        _action->setEnabled(FALSE);
        _delete->setEnabled(FALSE);
      }

      if(1 == lineMode || 4 == lineMode)
      {
        _issueStock->setEnabled(FALSE);
        _issueLineBalance->setEnabled(FALSE);
      }
    }
    else
    {
      _edit->setEnabled(FALSE);
      _action->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
    }
  }
  else
  {
    _edit->setEnabled(FALSE);
    _action->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _issueStock->setEnabled(FALSE);
    _issueLineBalance->setEnabled(FALSE);
  }
}

void transferOrder::sAction()
{
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    if (_lineMode == cClosed)
    {
      q.prepare( "UPDATE toitem "
                 "SET toitem_status=:toitem_status "
                 "WHERE (toitem_id=:toitem_id);" );
      q.bindValue(":toitem_status", _statusTypes[_status->currentIndex()]);
      q.bindValue(":toitem_id", _toitem->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else
    {
      q.prepare( "SELECT closeToItem(:toitem_id) AS result;" );
      q.bindValue(":toitem_id", _toitem->id());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("closeToItem", result),
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
    sFillItemList();

    q.prepare("SELECT tohead_status FROM tohead WHERE (tohead_id=:tohead_id);");
    q.bindValue(":tohead_id", _toheadid);
    q.exec();
    if (q.first())
    {
      if (q.value("tohead_status").toString() == "U")
        _status->setCurrentIndex(0);
      else if (q.value("tohead_status").toString() == "O")
        _status->setCurrentIndex(1);
      else if (q.value("tohead_status").toString() == "C")
        _status->setCurrentIndex(2);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrder::sDelete()
{ 
  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    if (QMessageBox::question(this, tr("Delete Selected Line Item?"),
                              tr("<p>Are you sure that you want to delete the "
                                 "selected Line Item?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      q.prepare( "DELETE FROM toitem "
                 "WHERE (toitem_id=:toitem_id);" );
      q.bindValue(":toitem_id", _toitem->id());
      q.exec();
      sFillItemList();

      if (_toitem->topLevelItemCount() == 0)
      {
        if (QMessageBox::question(this, tr("Cancel Transfer Order?"),
                                  tr("<p>You have deleted all of the Line "
                                     "Items for this Transfer Order. Would you "
                                     "like to cancel this Transfer Order?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        {
          q.prepare( "SELECT deleteTO(:tohead_id) AS result;");
          q.bindValue(":tohead_id", _toheadid);
          q.exec();
          if (q.first())
          {
            int result = q.value("result").toInt();
            if (result < 0)
              systemError(this, storedProcErrorLookup("deleteTO", result),
			  __FILE__, __LINE__);
          }
          else if (q.lastError().type() != QSqlError::NoError)
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

          clear();
          omfgThis->sTransferOrdersUpdated(_toheadid);
        }
      }
    }
  }
}

void transferOrder::populate()
{
  if ( (_mode == cEdit) || (_mode == cView) )
  {
    XSqlQuery to;
    to.prepare( "SELECT * "
                "FROM tohead "
                "WHERE (tohead_id=:tohead_id);" );
    to.bindValue(":tohead_id", _toheadid);
    to.exec();
    if (to.first())
    {
      _orderNumber->setText(to.value("tohead_number"));
      _orderNumber->setEnabled(FALSE);
      _orderDate->setDate(to.value("tohead_orderdate").toDate(), true);

      if (to.value("tohead_status").toString() == "U")
        _status->setCurrentIndex(0);
      else if (to.value("tohead_status").toString() == "O")
        _status->setCurrentIndex(1);
      else if (to.value("tohead_status").toString() == "C")
      {
        _status->setCurrentIndex(2);
        _status->setEnabled(false);
      }

      _srcWhs->setId(to.value("tohead_src_warehous_id").toInt());
      if (! _srcWhs->isValid())
        _srcWhs->setCode(to.value("tohead_srcname").toString());
      if (_srcAddr->line1() !=to.value("tohead_srcaddress1").toString() ||
          _srcAddr->line2() !=to.value("tohead_srcaddress2").toString() ||
          _srcAddr->line3() !=to.value("tohead_srcaddress3").toString() ||
          _srcAddr->city()  !=to.value("tohead_srccity").toString() ||
          _srcAddr->state() !=to.value("tohead_srcstate").toString() ||
          _srcAddr->postalCode()!=to.value("tohead_srcpostalcode").toString() ||
          _srcAddr->country()!=to.value("tohead_srccountry").toString() )
      {
        _srcAddr->setId(-1);
        _srcAddr->setLine1(to.value("tohead_srcaddress1").toString());
        _srcAddr->setLine2(to.value("tohead_srcaddress2").toString());
        _srcAddr->setLine3(to.value("tohead_srcaddress3").toString());
        _srcAddr->setCity(to.value("tohead_srccity").toString());
        _srcAddr->setState(to.value("tohead_srcstate").toString());
        _srcAddr->setPostalCode(to.value("tohead_srcpostalcode").toString());
        _srcAddr->setCountry(to.value("tohead_srccountry").toString());
      }
      _srcContact->setId(to.value("tohead_srccntct_id").toInt());
      if (_srcContact->name() != to.value("tohead_srccntct_name").toString() ||
	  _srcContact->phone() != to.value("tohead_srcphone").toString())
      {
	_srcContact->setId(-1);
	_srcContact->setObjectName(to.value("tohead_srccntct_name").toString());
	_srcContact->setPhone(to.value("tohead_srcphone").toString());
      }

      _trnsWhs->setId(to.value("tohead_trns_warehous_id").toInt());
      if (! _trnsWhs->isValid())
	_trnsWhs->setCode(to.value("tohead_trnsname").toString());

      _dstWhs->setId(to.value("tohead_dest_warehous_id").toInt());
      if (! _dstWhs->isValid())
	_dstWhs->setCode(to.value("tohead_destname").toString());
      if (_dstAddr->line1() !=to.value("tohead_destaddress1").toString() ||
          _dstAddr->line2() !=to.value("tohead_destaddress2").toString() ||
          _dstAddr->line3() !=to.value("tohead_destaddress3").toString() ||
          _dstAddr->city()  !=to.value("tohead_destcity").toString() ||
          _dstAddr->state() !=to.value("tohead_deststate").toString() ||
          _dstAddr->postalCode()!=to.value("tohead_destpostalcode").toString() ||
          _dstAddr->country()!=to.value("tohead_destcountry").toString() )
      {
        _dstAddr->setId(-1);
        _dstAddr->setLine1(to.value("tohead_destaddress1").toString());
        _dstAddr->setLine2(to.value("tohead_destaddress2").toString());
        _dstAddr->setLine3(to.value("tohead_destaddress3").toString());
        _dstAddr->setCity(to.value("tohead_destcity").toString());
        _dstAddr->setState(to.value("tohead_deststate").toString());
        _dstAddr->setPostalCode(to.value("tohead_destpostalcode").toString());
        _dstAddr->setCountry(to.value("tohead_destcountry").toString());
      }
      _dstContact->setId(to.value("tohead_destcntct_id").toInt());
      if (_dstContact->name() != to.value("tohead_destcntct_name").toString() ||
        _dstContact->phone() != to.value("tohead_destphone").toString())
      {
        _dstContact->setId(-1);
        _dstContact->setObjectName(to.value("tohead_destcntct_name").toString());
        _dstContact->setPhone(to.value("tohead_destphone").toString());
      }

      _agent->setText(to.value("tohead_agent_username").toString());
      _shipVia->setText(to.value("tohead_shipvia").toString());
      _shippingForm->setId(to.value("tohead_shipform_id").toInt());
      _taxzoneidCache = to.value("tohead_taxzone_id").toInt();
      _taxzone->setId(to.value("tohead_taxzone_id").toInt());

      _freight->setId(to.value("tohead_freight_curr_id").toInt());
      _freight->setLocalValue(to.value("tohead_freight").toDouble());

      if (to.value("tohead_freighttax_id").isNull())

      _shipComplete->setChecked(to.value("tohead_shipcomplete").toBool());
      _orderComments->setText(to.value("tohead_ordercomments").toString());
      _shippingComments->setText(to.value("tohead_shipcomments").toString());
      _packDate->setDate(to.value("tohead_packdate").toDate());
      _project->setId(to.value("tohead_prj_id").toInt());

      _comments->setId(_toheadid);
      sFillItemList();
    }
    else if (to.lastError().type() != QSqlError::NoError)
    {
      systemError(this, to.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrder::sFillItemList()
{
  q.prepare( "SELECT MIN(toitem_schedshipdate) AS shipdate "
	     "FROM toitem "
	     "WHERE ((toitem_status <> 'X')"
	     "  AND  (toitem_tohead_id=:head_id));" );

  q.bindValue(":head_id", _toheadid);
  q.exec();
  if (q.first())
  {
    _shipDate->setDate(q.value("shipdate").toDate());

    if (cNew == _mode)
      _packDate->setDate(q.value("shipdate").toDate());
  }
  else
  {
    _shipDate->clear();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  _toitem->clear();
  if (_mode == cEdit || _mode == cNew)
  {
    _srcWhs->setEnabled(true);
    _trnsWhs->setEnabled(true);
    _dstWhs->setEnabled(true);
  }

  QString sql("SELECT toitem_id, closestatus,"
	      "       toitem_status,"
	      "       toitem_linenumber, item_number, description,"
	      "       toitem_schedshipdate,"
	      "       toitem_qty_ordered, toitem_qty_shipped, toitem_qty_received,"
        "       balance, atshipping, tagged, in_future, enabletoshipping, backorder,"
        "       CASE WHEN (backorder AND toitem_status NOT IN ('C', 'X')) THEN 'red' "
        "            WHEN (enabletoshipping AND tagged AND in_future) THEN 'darkgreen' "
        "            WHEN (enabletoshipping AND tagged) THEN 'red' "
        "       END AS qtforegroundrole,"
        "       'qty' AS toitem_qty_ordered_xtnumericrole,"
        "       'qty' AS toitem_qty_shipped_xtnumericrole,"
        "       'qty' AS toitem_qty_received_xtnumericrole,"
        "       'qty' AS atshipping_xtnumericrole,"
        "       'qty' AS balance_xtnumericrole "
        "      FROM ( "
        "SELECT toitem_id,"
	      "       toitem_status,"
	      "       CASE WHEN (toitem_status='C') THEN 1"
	      "            WHEN (toitem_status='X') THEN 4"
	      "            WHEN (toitem_status='U') THEN 5"
	      "            WHEN ((toitem_status='O')"
	      "                  AND ((qtyAtShipping('TO', toitem_id) > 0) OR"
	      "                          (toitem_qty_shipped > 0) ) ) THEN 2"
	      "            ELSE 3"
	      "       END AS closestatus,"
	      "       toitem_linenumber, item_number,"
	      "       (item_descrip1 || ' ' || item_descrip2) AS description,"
	      "       toitem_schedshipdate,"
	      "       toitem_qty_ordered, toitem_qty_shipped, toitem_qty_received, "
	      "       noNeg(toitem_qty_ordered - toitem_qty_shipped) AS balance,"
	      "       qtyAtShipping('TO', toitem_id) AS atshipping,"
	      "       (noNeg(toitem_qty_ordered - toitem_qty_shipped) <> qtyAtShipping('TO', toitem_id)) AS tagged,"
	      "       CASE WHEN toitem_schedshipdate > CURRENT_DATE THEN true"
	      "            ELSE false"
	      "       END AS in_future,"
        "       fetchMetricBool('EnableTOShipping') AS enabletoshipping,"
        "       ((SELECT COALESCE(toitem_id, -1) FROM toitem WHERE ((toitem_status='C') AND (toitem_tohead_id=tohead_id)) LIMIT 1) <> -1) AS backorder " 
	      "FROM item, toitem, tohead "
	      "WHERE ((toitem_item_id=item_id)"
        "  AND  (tohead_id=toitem_tohead_id)"
	      "<? if exists(\"showCanceled\") ?>"
	      "<? else ?>"
	      "  AND  (toitem_status != 'X')"
	      "<? endif ?>"
	      "  AND  (toitem_tohead_id=<? value(\"tohead_id\") ?>) ) ) AS data "
	      "ORDER BY toitem_linenumber;" );

  ParameterList params;
  params.append("tohead_id", _toheadid);
  if (_showCanceled->isChecked())
    params.append("showCanceled");
	     
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _toitem->populate(q, true);
  if (q.first())
  {
    do
    {
      if (q.value("toitem_qty_received").toDouble() > 0)
      {
	_dstWhs->setEnabled(false);
	_trnsWhs->setEnabled(false);
	_srcWhs->setEnabled(false);
      }
      else if (q.value("toitem_qty_shipped").toDouble() > 0)
      {
	_trnsWhs->setEnabled(false);
	_srcWhs->setEnabled(false);
      }
      else if (q.value("atshipping").toDouble() > 0)
      {
	_srcWhs->setEnabled(false);
      }
    }
    while (q.next());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT formatQty(SUM(COALESCE(toitem_qty_ordered, 0.00) *"
	    "                 (COALESCE(item_prodweight, 0.00) +"
	    "                  COALESCE(item_packweight, 0.00)))) AS grossweight,"
	    "       SUM(toitem_freight) AS linefreight "
	    "FROM toitem, item "
	    "WHERE ((toitem_item_id=item_id)"
	    " AND (toitem_status<>'X')"
	    " AND (toitem_tohead_id=:head_id));");

  q.bindValue(":head_id", _toheadid);
  q.exec();
  if (q.first())
  {
    _weight->setText(q.value("grossweight").toDouble());
    _itemFreight->setLocalValue(q.value("linefreight").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sCalculateTax(); // triggers sCalculateTotal();

  _srcWhs->setEnabled(_toitem->topLevelItemCount() == 0);
  _freightCurrency->setEnabled(_toitem->topLevelItemCount() == 0);
  _qecurrency->setEnabled(_toitem->topLevelItemCount() == 0);

  _qeitem->select();
}

void transferOrder::sCalculateTotal()
{
  _total->setLocalValue(_tax->localValue() + _freight->localValue() +
			_itemFreight->localValue());
}

bool transferOrder::deleteForCancel()
{
  XSqlQuery query;
  bool deleteTo = false;

  if (cNew == _mode && _toitem->topLevelItemCount()== 0)
  {
    deleteTo = true;
  }

  if (cNew == _mode && _toitem->topLevelItemCount() > 0)
  {
    if (QMessageBox::question(this, tr("Delete Transfer Order?"),
                          tr("<p>Are you sure you want to delete this "
                             "Transfer Order and its associated Line Items?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return false;
    else
      deleteTo = true;
  }

  if (cNew != _mode && _toitem->topLevelItemCount() == 0)
  {
    if (QMessageBox::question(this, tr("Delete Transfer Order?"),
			                  tr("<p>This Transfer Order does not have any line items.  "
				                 "Are you sure you want to delete this Transfer Order?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return false;
    else
      deleteTo = true;
  }

  if (deleteTo)
  {
    query.prepare("SELECT deleteTO(:tohead_id) AS result;");
    query.bindValue(":tohead_id", _toheadid);
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("deleteSO", result),
                    __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(cNew == _mode)
    sReleaseTohead();

  return true;
}

void transferOrder::sClear()
{
  if (! deleteForCancel())
    return;
  clear();
}

void transferOrder::clear()
{
  if (cView != _mode)
    sReleaseTohead();
  _toheadid = -1;

  _tabs->setCurrentIndex(0);

  _orderNumber->setEnabled(TRUE);
  _orderNumberGen = 0;
  _orderNumber->clear();

  _shipDate->clear();
  _packDate->clear();
  _srcWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _trnsWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());
  _dstWhs->setId(_preferences->value("PreferredWarehouse").toInt());
  _status->setCurrentIndex(0);
  _agent->setCurrentIndex(-1);
  _taxzoneidCache = -1;
  _taxzone->setId(-1);
  _whstaxzoneid        = -1;
  _shipVia->setId(-1);
//  cannot clear _shippingForm
//  _shippingForm->setId(-1);
  _freight->clear();
  _orderComments->clear();
  _shippingComments->clear();
  _tax->clear();
  _freight->clear();
  _total->clear();
  _weight->clear();
  _project->setId(-1);
  _srcWhs->setEnabled(true);
  _freightCurrency->setEnabled(true);
  _qecurrency->setEnabled(true);
  _qeitem->removeRows(0, _qeitem->rowCount());

  _shipComplete->setChecked(false);

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    _mode = cNew;
    setObjectName("transferOrder new");
    _orderDate->setDate(omfgThis->dbDate(), true);
  }

  if ( (_metrics->value("TONumberGeneration") == "A") ||
       (_metrics->value("TONumberGeneration") == "O")   )
  {
    if (! insertPlaceholder())
      return;
  }
  
  if (_orderNumber->text().isEmpty())
    _orderNumber->setFocus();
  else
    _srcWhs->setFocus();

  _toitem->clear();

  _saved = false;
}

void transferOrder::closeEvent(QCloseEvent *pEvent)
{
  // TODO: if sQeSave == false then find a way to return control to the user
  if (_qeitem->isDirty())
  {
    if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
		    tr("Do you want to save your Quick Entry changes?"),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      sQESave();
  }

  if (! deleteForCancel())
  {
    pEvent->ignore();
    return;
  }

  disconnect(_orderNumber,    SIGNAL(lostFocus()), this, SLOT(sHandleOrderNumber()));

  sReleaseTohead();
  
  if (cView != _mode)
    omfgThis->sTransferOrdersUpdated(-1);

  XWidget::closeEvent(pEvent);
}

void transferOrder::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) )
    _freight->setEnabled(FALSE);
  else
  {
    XSqlQuery query;
    query.prepare( "SELECT shipchrg_custfreight "
                   "FROM shipchrg "
                   "WHERE (shipchrg_id=:shipchrg_id);" );
    query.bindValue(":shipchrg_id", pShipchrgid);
    query.exec();
    if (query.first())
    {
      if (query.value("shipchrg_custfreight").toBool())
        _freight->setEnabled(TRUE);
      else
      {
        _freight->setEnabled(FALSE);
        _freight->clear();
      }
    }
  }
}

void transferOrder::sHandleTransferOrderEvent(int pToheadid)
{
  if (pToheadid == _toheadid)
    sFillItemList();
}

void transferOrder::sTaxDetail()
{
  XSqlQuery taxq;
  if (cView != _mode)
  {
    taxq.prepare("UPDATE tohead SET tohead_taxzone_id=:taxzone, "
		  "  tohead_freight=:freight,"
		  "  tohead_orderdate=:date "
		  "WHERE (tohead_id=:head_id);");
    if (_taxzone->isValid())
      taxq.bindValue(":taxzone", _taxzone->id());
    taxq.bindValue(":freight", _freight->localValue());
    taxq.bindValue(":date",    _orderDate->date());
    taxq.bindValue(":head_id", _toheadid);
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("order_id", _toheadid);
  params.append("order_type", "TO");
  params.append("mode", "view"); // because tohead has no fields to hold changes

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    populate();
  }
}

void transferOrder::setViewMode()
{
  if(cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    _captive = false;
  }
  _new->setEnabled(false);

  disconnect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sHandleTransferOrderEvent(int)));

  _mode = cView;
  setObjectName(QString("transferOrder view %1").arg(_toheadid));

  _orderNumber->setEnabled(FALSE);
  _status->setEnabled(FALSE);
  _packDate->setEnabled(FALSE);
  _srcWhs->setEnabled(FALSE);
  _dstWhs->setEnabled(FALSE);
  _trnsWhs->setEnabled(FALSE);
  _agent->setEnabled(FALSE);
  _taxzone->setEnabled(FALSE);
  _shipVia->setEnabled(FALSE);
  _shippingForm->setEnabled(FALSE);
  _freight->setEnabled(FALSE);
  _orderComments->setEnabled(FALSE);
  _shippingComments->setEnabled(FALSE);
  _qeitemView->setEnabled(FALSE);
  _qesave->setEnabled(FALSE);
  _qedelete->setEnabled(FALSE);
  _qecurrency->setEnabled(FALSE);
  _freightCurrency->setEnabled(FALSE);
  _srcContact->setEnabled(FALSE);
  _dstContact->setEnabled(FALSE);

  _edit->setText(tr("View"));
  _comments->setReadOnly(true);
  _shipComplete->setEnabled(false);
  _save->hide();
  _clear->hide();
  _issueStock->hide();
  _issueLineBalance->hide();
  _project->setReadOnly(true);
  if(_metrics->boolean("AlwaysShowSaveAndAdd"))
    _saveAndAdd->setEnabled(false);
  else
    _saveAndAdd->hide();
  _action->hide();
  _delete->hide();
}

void transferOrder::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_N && (e->modifiers() & Qt::ControlModifier))
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && (e->modifiers() & Qt::ControlModifier))
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void transferOrder::newTransferOrder(int pSrcWhsid, int pDstWhsid)
{
  // Check for an Item window in new mode already.
  if(pSrcWhsid == -1 && pDstWhsid == -1)
  {
    QWidgetList list = omfgThis->windowList();
    for(int i = 0; i < list.size(); i++)
    {
      QWidget * w = list.at(i);
      if(QString::compare(w->objectName(), "transferOrder new")==0)
      {
        w->setFocus();
        if(omfgThis->showTopLevel())
        {
          w->raise();
          w->activateWindow();
        }
        return;
      }
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");
  if(pSrcWhsid != -1)
    params.append("src_warehous_id", pSrcWhsid);
  if (pDstWhsid != -1)
    params.append("dst_warehous_id", pDstWhsid);

  transferOrder *newdlg = new transferOrder();
  if(newdlg->set(params) != UndefinedError)
    omfgThis->handleNewWindow(newdlg);
  else
    delete newdlg;
}

void transferOrder::editTransferOrder( int pId , bool enableSaveAndAdd )
{
  // Check for an Item window in edit mode for the specified transferOrder already.
  QString n = QString("transferOrder edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "edit");
  params.append("tohead_id", pId);
  if(enableSaveAndAdd)
    params.append("enableSaveAndAdd");

  transferOrder *newdlg = new transferOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void transferOrder::viewTransferOrder( int pId )
{
  // Check for an Item window in view mode for the specified transferOrder already.
  QString n = QString("transferOrder view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "view");
  params.append("tohead_id", pId);

  transferOrder *newdlg = new transferOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void transferOrder::sReturnStock()
{
  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT returnItemShipments('TO', :toitem_id, 0, CURRENT_TIMESTAMP) AS result;");
  QList<XTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.bindValue(":toitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("returnItemShipments", result) +
                          tr("<br>Line Item %1").arg(selected[i]->text(0)),
                           __FILE__, __LINE__);
        return;
      }
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Return Stock"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                        q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillItemList();
}

void transferOrder::sIssueStock()
{
  bool update  = FALSE;
  QList<XTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem* toitem = (XTreeWidgetItem*)(selected[i]);
    // skip if status = C or X or U
    if (toitem->altId() != 1 && toitem->altId() != 4 && toitem->altId() != 5)
    {
      ParameterList params;
      params.append("toitem_id", toitem->id());

      if(_requireInventory->isChecked())
        params.append("requireInventory");

      issueLineToShipping newdlg(this, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
        update = TRUE;
    }
  }

  if (update)
    sFillItemList();
}

void transferOrder::sIssueLineBalance()
{
  QList<XTreeWidgetItem*> selected = _toitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem* toitem = (XTreeWidgetItem*)(selected[i]);
    // skip if status = C or X or U
    if (toitem->altId() != 1 && toitem->altId() != 4 && toitem->altId() != 5)
    {

      if(_requireInventory->isChecked())
      {
        q.prepare("SELECT sufficientInventoryToShipItem('TO', :toitem_id) AS result;");
        q.bindValue(":toitem_id", toitem->id());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            q.prepare("SELECT item_number, tohead_srcname "
                "  FROM toitem, tohead, item "
                " WHERE ((toitem_item_id=item_id)"
                "   AND  (toitem_tohead_id=tohead_id)"
                "   AND  (toitem_id=:toitem_id)); ");
            q.bindValue(":toitem_id", toitem->id());
            q.exec();
            if (! q.first() && q.lastError().type() != QSqlError::NoError)
            {
              systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
                systemError(this,
                storedProcErrorLookup("sufficientInventoryToShipItem",
                          result)
                .arg(q.value("item_number").toString())
                .arg(q.value("tohead_srcname").toString()), __FILE__, __LINE__);
              return;
            }
          }
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }

      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
      q.prepare("SELECT issueLineBalanceToShipping('TO', :toitem_id, CURRENT_TIMESTAMP) AS result;");
      q.bindValue(":toitem_id", toitem->id());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError(this, storedProcErrorLookup("issueLineBalance", result) +
                            tr("<br>Line Item %1").arg(selected[i]->text(0)),
                      __FILE__, __LINE__);
          return;
        }
        if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Issue to Shipping"), tr("Transaction Canceled") );
          return;
        }

        q.exec("COMMIT;");
      }
      else
      {
        rollback.exec();
        systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                          q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  sFillItemList();
}

void transferOrder::sCalculateTax()
{
  XSqlQuery taxq;

  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(calculateTax(:taxzone_id,getFreightTaxTypeId(),:date,:curr_id,toitem_freight),2) AS tax "
                "FROM toitem "
                "WHERE (toitem_tohead_id=:tohead_id) "
                "UNION ALL "
                "SELECT ROUND(calculateTax(:taxzone_id,getFreightTaxTypeId(),:date,:curr_id,:freight),2) AS tax "
                ") AS data;" );
  taxq.bindValue(":tohead_id", _toheadid);
  taxq.bindValue(":taxzone_id", _taxzone->id());
  taxq.bindValue(":date", _orderDate->date());
  taxq.bindValue(":freight", _freight->localValue());
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  sCalculateTotal();
}

bool transferOrder::sQESave()
{
  _qesave->setFocus();
  if (! _qeitem->submitAll())
  {
    if (! _qeitem->lastError().databaseText().isEmpty())
      systemError(this, _qeitem->lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  sFillItemList();
  return true;
}

void transferOrder::sQEDelete()
{
  if (! _qeitem->removeRow(_qeitemView->currentIndex().row()))
  {
    systemError(this, tr("Removing row from view failed"), __FILE__, __LINE__);
    return;
  }
}

void transferOrder::sCurrencyChanged()
{
  // The user can only set the QE currency from the QE tab
  if (_tabs->currentWidget()->isAncestorOf(_qecurrency) &&
      _freightCurrency->id() != _qecurrency->id())
    _freightCurrency->setId(_qecurrency->id());
  else
    _qecurrency->setId(_freightCurrency->id());

  static_cast<ToitemTableModel*>(_qeitemView->model())->setCurrId(_freightCurrency->id());
}

void transferOrder::sTabChanged(int pIndex)
{
  if (pIndex != _cachedTabIndex)
  {
    if (pIndex == _tabs->indexOf(_qeTab) && ! _shipDate->isValid() && ! _packDate->isValid())
    {
      QMessageBox::warning(this, tr("Quick Entry Requires a Date"),
			   tr("<p>You must enter either a Scheduled Date or a "
			      "Pack Date before using the Quick Entry tab."));
      _cachedTabIndex = 0;
      _tabs->setCurrentIndex(0);
      if (_shipDate->isEnabled())
	_shipDate->setFocus();
      else if (_packDate->isEnabled())
	_packDate->setFocus();
    }
    else if (_cachedTabIndex == _tabs->indexOf(_qeTab) && _qeitem->isDirty())
    {
      if (QMessageBox::question(this, tr("Save Quick Entry Data?"),
		      tr("Do you want to save your Quick Entry changes?"),
		      QMessageBox::Yes | QMessageBox::Default,
		      QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
	if (! sQESave())
	{
	  _tabs->setCurrentIndex(_cachedTabIndex);
	  return;
	}
      _cachedTabIndex = pIndex;
    }
    else
      _cachedTabIndex = pIndex;
  }
}

void transferOrder::getWhsInfo(const int pid, const QWidget* pwidget)
{
  XSqlQuery whsq;
  whsq.prepare("SELECT * FROM whsinfo WHERE warehous_id=:id;");
  whsq.bindValue(":id", pid);
  whsq.exec();
  if (whsq.first())
  {
    if (pwidget == _srcWhs)
    {
      _srcContact->setId(whsq.value("warehous_cntct_id").toInt());
      _srcAddr->setId(whsq.value("warehous_addr_id").toInt());
      _qeitem->setSrcWhsId(pid);
    }
    else if (pwidget == _dstWhs)
    {
      _dstContact->setId(whsq.value("warehous_cntct_id").toInt());
      _dstAddr->setId(whsq.value("warehous_addr_id").toInt());
      _taxzone->setId(whsq.value("warehous_taxzone_id").toInt());
    }
    else if (pwidget == _trnsWhs)
    {
      _shippingForm->setId(whsq.value("warehous_shipform_id").toInt());
      _shipVia->setId(whsq.value("warehous_shipvia_id").toInt());
      _shippingComments->setText(whsq.value("warehous_shipcomments").toString());
    }
  }
  else if (whsq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, whsq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrder::sHandleDstWhs(const int pid)
{
  getWhsInfo(pid, _dstWhs);
}

void transferOrder::sHandleSrcWhs(const int pid)
{
  getWhsInfo(pid, _srcWhs);
}

void transferOrder::sHandleTrnsWhs(const int pid)
{
  getWhsInfo(pid, _trnsWhs);
}

void transferOrder::sReleaseTohead()
{
  if (!_locked)
    return;

  XSqlQuery query;
  query.prepare("SELECT pg_advisory_unlock(oid::integer, :tohead_id) AS result "
                "FROM pg_class "
                "WHERE relname='tohead';");
  query.bindValue(":tohead_id", _toheadid);
  query.exec();
  if (query.first() && ! query.value("result").toBool())
    systemError(this, tr("Could not release this Transfer Order record."),
                __FILE__, __LINE__);
  else if (query.lastError().type() != QSqlError::NoError)
    systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
  else
    _locked = false;
}
