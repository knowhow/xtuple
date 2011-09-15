/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// TODO: rename unpostedReceipts
#include "unpostedPoReceipts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "distributeInventory.h"
#include "enterPoReceipt.h"
#include "enterPoitemReceipt.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "mqlutil.h"
#include "purchaseOrderItem.h"
#include "storedProcErrorLookup.h"
#include "transferOrderItem.h"
#include "returnAuthorizationItem.h"

unpostedPoReceipts::unpostedPoReceipts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,	          SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_post,          SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print,         SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_recv, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_viewOrderItem,    SIGNAL(clicked()), this, SLOT(sViewOrderItem()));
  connect(omfgThis, SIGNAL(purchaseOrderReceiptsUpdated()), this, SLOT(sFillList()));

  _recv->addColumn(tr("Order #"),       _orderColumn, Qt::AlignRight,  true, "recv_order_number"  );
  _recv->addColumn(tr("Type"),          50,           Qt::AlignCenter, true, "recv_order_type" );
  _recv->addColumn(tr("From"),          -1,           Qt::AlignLeft,   true, "orderhead_from"   );
  _recv->addColumn(tr("Line #"),        50,           Qt::AlignRight,  true, "orderitem_linenumber");
  _recv->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter, true, "recv_duedate");
  _recv->addColumn(tr("Site"),          _whsColumn,   Qt::AlignRight,  true, "warehous_code"  );
  _recv->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignRight,  true, "item_number");
  _recv->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true, "uom_name");
  _recv->addColumn(tr("Vend. Item #"),  _itemColumn,  Qt::AlignLeft,   true, "recv_vend_item_number");
  _recv->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true, "recv_vend_uom");
  _recv->addColumn(tr("Ordered"),       _qtyColumn,   Qt::AlignRight,  true, "orderitem_qty_ordered");
  _recv->addColumn(tr("Received"),      _qtyColumn,   Qt::AlignRight,  true, "orderitem_qty_received");
  _recv->addColumn(tr("To Receive"),    _qtyColumn,   Qt::AlignRight,  true, "recv_qty");
  _recv->addColumn(tr("Receipt Date"),  _dateColumn,  Qt::AlignCenter, true, "recv_date");
  _recv->addColumn(tr("G/L Post Date"), _dateColumn,  Qt::AlignCenter, true, "recv_gldistdate");

  if (! _privileges->check("ChangePORecvPostDate"))
    _recv->hideColumn(_recv->column("recv_gldistdate"));

  if(!_privileges->check("ViewPurchaseOrders"))
    disconnect(_recv, SIGNAL(valid(bool)), _viewOrderItem, SLOT(setEnabled(bool)));

  sFillList();
}

unpostedPoReceipts::~unpostedPoReceipts()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedPoReceipts::languageChange()
{
  retranslateUi(this);
}

void unpostedPoReceipts::setParams(ParameterList & params)
{
  params.append("nonInventory",	tr("Non-Inventory"));
  params.append("na",		tr("N/A"));
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth");
}

void unpostedPoReceipts::sPrint()
{
  ParameterList params;
  setParams(params);
  orReport report("UnpostedPoReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unpostedPoReceipts::sNew()
{
  ParameterList params;

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPoReceipts::sEdit()
{
  ParameterList params;
  params.append("mode",		"edit");
  params.append("recv_id",	_recv->id());

  enterPoitemReceipt *newdlg = new enterPoitemReceipt();
  newdlg->set(params);
  newdlg->exec();
}

void unpostedPoReceipts::sDelete()
{
  if (QMessageBox::question(this, tr("Cancel Receipts?"),
			    tr("<p>Are you sure you want to delete these "
			       "unposted Receipts?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM recv "
	       "WHERE (recv_id IN (:id));" );
    QList<XTreeWidgetItem*>selected = _recv->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":id", ((XTreeWidgetItem*)(selected[i]))->id() );
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    omfgThis->sPurchaseOrderReceiptsUpdated();
  }
}

void unpostedPoReceipts::sViewOrderItem()
{
  ParameterList params;
  params.append("mode",		"view");

  QString ordertype = _recv->currentItem()->text(_recv->column("recv_order_type"));
  if (ordertype == "PO")
  {
    params.append("poitem_id",	_recv->altId());
    purchaseOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (ordertype == "TO")
  {
    params.append("toitem_id",	_recv->altId());
    transferOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (ordertype == "RA")
  {
    params.append("raitem_id",	_recv->altId());
    returnAuthorizationItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void unpostedPoReceipts::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangePORecvPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Receipt Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE recv SET recv_gldistdate=:distdate "
		  "WHERE recv_id=:recv_id;");

  QList<XTreeWidgetItem*>selected = _recv->selectedItems();
  QList<XTreeWidgetItem*>triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    int id = ((XTreeWidgetItem*)(selected[i]))->id();

    if (changeDate)
    {
      setDate.bindValue(":distdate",  newDate);
      setDate.bindValue(":recv_id", id);
      setDate.exec();
      if (setDate.lastError().type() != QSqlError::NoError)
      {
        systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }
  
  XSqlQuery postLine;
  postLine.prepare("SELECT postReceipt(:id, NULL::integer) AS result, "
                   "  (recv_order_type = 'RA' AND COALESCE(itemsite_costmethod, '') = 'J') AS issuewo, "
                   "  COALESCE(pohead_dropship, false) AS dropship "
                   "FROM recv "
                   "  LEFT OUTER JOIN itemsite ON (itemsite_id=recv_itemsite_id) "
                   "  LEFT OUTER JOIN poitem ON ((recv_order_type='PO') "
                   "                         AND (recv_orderitem_id=poitem_id)) "
                   "  LEFT OUTER JOIN pohead ON (poitem_pohead_id=pohead_id) "
                   "WHERE (recv_id=:id);");
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  bool tryagain = false;
  do {
    q.exec("BEGIN;");

    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      postLine.bindValue(":id", id);
      postLine.exec();
      if (postLine.first())
      {
        int result = postLine.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("postReceipt", result),
              __FILE__, __LINE__);
          rollback.exec();
          continue;
        }

        if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Unposted Receipts"), tr("Post Canceled") );
          return;
        }

        // Job item for Return Service; issue this to work order
        if (postLine.value("issuewo").toBool())
        {
          XSqlQuery issuewo;
          issuewo.prepare("SELECT issueWoRtnReceipt(coitem_order_id, invhist_id) AS result "
                          "FROM invhist, recv "
                          " JOIN raitem ON (raitem_id=recv_orderitem_id) "
                          " JOIN coitem ON (coitem_id=raitem_new_coitem_id) "
                          "WHERE ((invhist_series=:itemlocseries) "
                          " AND (recv_id=:id));");
          issuewo.bindValue(":itemlocseries", postLine.value("result").toInt());
          issuewo.bindValue(":id", id);
          issuewo.exec();
          if (issuewo.lastError().type() != QSqlError::NoError)
          {
            systemError(this, issuewo.lastError().databaseText(), __FILE__, __LINE__);
            rollback.exec();
            return;
          }
        }
        // Issue drop ship orders to shipping
        else if (postLine.value("dropship").toBool())
        {
          XSqlQuery issue;
          issue.prepare("SELECT issueToShipping('SO', coitem_id, recv_qty, "
                        "  :itemlocseries, now(), invhist_id ) AS result, "
                        "  coitem_cohead_id, cohead_holdtype, pohead_number "
                        "FROM invhist, recv "
                        " JOIN poitem ON (poitem_id=recv_orderitem_id) "
                        " JOIN pohead ON (poitem_pohead_id=pohead_id) "
                        " JOIN coitem ON (coitem_id=poitem_soitem_id) "
                        " JOIN cohead ON (coitem_cohead_id=cohead_id) "
                        "WHERE ((invhist_series=:itemlocseries) "
                        " AND (recv_id=:id));");
          issue.bindValue(":itemlocseries", postLine.value("result").toInt());
          issue.bindValue(":id",  id);
          issue.exec();
          if (issue.first())
          {
            if (issue.value("cohead_holdtype").toString() != "N")
            {
              QString msg = tr("Purchase Order %1 is being drop shipped against "
                       "a Sales Order that is on Hold.  The Sales Order must "
                       "be taken off Hold before the Receipt may be Posted.")
                  .arg(issue.value("pohead_number").toString());
              rollback.exec();
              QMessageBox::warning(this, tr("Cannot Ship Order"), msg);
              sFillList();
              return;
            }

            if (!_soheadid.contains(issue.value("coitem_cohead_id").toInt()))
              _soheadid.append(issue.value("coitem_cohead_id").toInt());
            issue.prepare("SELECT postItemLocSeries(:itemlocseries);");
            issue.bindValue(":itemlocseries", postLine.value("result").toInt());
            issue.exec();
          }
          if (issue.lastError().type() != QSqlError::NoError)
          {
            systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
            rollback.exec();
            return;
          }
        }    
      }
      // contains() string is hard-coded in stored procedure
      else if (postLine.lastError().databaseText().contains("posted to closed period"))
      {
        if (changeDate)
        {
          triedToClosed = selected;
          break;
        }
        else
          triedToClosed.append(selected[i]);
      }
      else if (postLine.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, postLine.lastError().databaseText(), __FILE__, __LINE__);
      }
    } // for each selected line

    // Ship any drop shipped orders
    while (_soheadid.count())
    {
      XSqlQuery ship;
      ship.prepare("SELECT shipShipment(shiphead_id) AS result, "
                   "  shiphead_id "
                   "FROM shiphead "
                   "WHERE ( (shiphead_order_type='SO') "
                   " AND (shiphead_order_id=:cohead_id) "
                   " AND (NOT shiphead_shipped) );");
      ship.bindValue(":cohead_id", _soheadid.at(0));
      ship.exec();
      if (_metrics->boolean("BillDropShip") && ship.first())
      {
        int shipheadid = ship.value("shiphead_id").toInt();
        ship.prepare("SELECT selectUninvoicedShipment(:shiphead_id);");
        ship.bindValue(":shiphead_id", shipheadid);
        ship.exec();
        if (ship.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, ship.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
      if (ship.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, ship.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      _soheadid.takeFirst();
    }

    q.exec("COMMIT;");

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _recv->headerItem(), _recv->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  omfgThis->sPurchaseOrderReceiptsUpdated();
}

void unpostedPoReceipts::sPopulateMenu(QMenu *pMenu,QTreeWidgetItem *pItem)
{
  QAction *menuItem;
  QString ordertype = pItem->text(_recv->column("recv_order_type"));

  menuItem = pMenu->addAction(tr("Edit Receipt..."),	this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("Delete Receipt..."),	this, SLOT(sDelete()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post Receipt..."),	this, SLOT(sPost()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Order Item..."),this, SLOT(sViewOrderItem()));
  menuItem->setEnabled(
      (ordertype == "PO" && _privileges->check("ViewPurchaseOrders")) ||
      (ordertype == "TO" && _privileges->check("ViewTransferOrders")) ||
      (ordertype == "RA" && _privileges->check("ViewReturns"))
    );
}

void unpostedPoReceipts::sFillList()
{
  ParameterList fillp;
  setParams(fillp);
  MetaSQLQuery fillm = mqlLoad("unpostedReceipts", "detail");
  XSqlQuery fillq = fillm.toQuery(fillp);

  _recv->clear();
  _recv->populate(fillq,true);
}
