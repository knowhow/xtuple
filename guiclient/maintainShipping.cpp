/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "maintainShipping.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "distributeInventory.h"
#include "mqlutil.h"
#include "shippingInformation.h"
#include "shipOrder.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "transferOrder.h"
#include "transferOrderItem.h"
#include "printShippingForm.h"
#include "issueToShipping.h"
#include "storedProcErrorLookup.h"

maintainShipping::maintainShipping(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_ship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _ship->setRootIsDecorated(TRUE);
  _ship->addColumn(tr("Shipment #"),       _orderColumn,  Qt::AlignLeft,  true, "shiphead_number");
  _ship->addColumn(tr("Order Type"),       _statusColumn, Qt::AlignLeft,  true, "ordertype");
  _ship->addColumn(tr("Order/Line #"),     _itemColumn,   Qt::AlignRight, true, "linenumber");
  _ship->addColumn(tr("Printed"),          _dateColumn,   Qt::AlignCenter,true, "sfstatus");
  _ship->addColumn(tr("Cust./Item #"),     _itemColumn,   Qt::AlignLeft,  true, "dest");
  _ship->addColumn(tr("Name/Description"), -1,            Qt::AlignLeft,  true, "description");
  _ship->addColumn(tr("Ship Via"),         _itemColumn,   Qt::AlignLeft,  true, "shiphead_shipvia");
  _ship->addColumn(tr("UOM"),              _uomColumn,    Qt::AlignLeft,  true, "uom_name");
  _ship->addColumn(tr("Qty. At Ship"),     _qtyColumn,    Qt::AlignRight, true, "shipqty");
  _ship->addColumn(tr("Value At Ship"),    _moneyColumn,  Qt::AlignRight, true, "shipvalue");
  _ship->addColumn(tr("Hold Type"),        _statusColumn, Qt::AlignCenter,true, "holdtype");

}

maintainShipping::~maintainShipping()
{
  // no need to delete child widgets, Qt does it all for us
}

void maintainShipping::languageChange()
{
  retranslateUi(this);
}

void maintainShipping::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  QAction *menuItem;

  if (selected->text(0) != "")
    _itemtype = 1;
  else if (selected->text(2) != "")
    _itemtype = 2;
  else
    _itemtype = 3;

  switch (_itemtype)
  {
    case 1:
      menuItem = pMenu->addAction("Shipping Information...", this, SLOT(sShippingInformation()));
      menuItem->setEnabled(_privileges->check("EnterShippingInformation"));

      menuItem = pMenu->addAction("Return ALL Stock Issued to Order...", this, SLOT(sReturnAllOrderStock()));
      menuItem->setEnabled(_privileges->check("ReturnStockFromShipping"));

      menuItem = pMenu->addAction("View Order...", this, SLOT(sViewOrder()));
      menuItem->setEnabled(_privileges->check("ViewSalesOrders"));

      menuItem = pMenu->addAction("Print Shipping Form...", this, SLOT(sPrintShippingForm()));
      menuItem->setEnabled(_privileges->check("PrintBillsOfLading"));

      if (selected->text(6) != "S")
      {
        menuItem = pMenu->addAction("Ship Order...", this, SLOT(sShipOrder()));
        menuItem->setEnabled(_privileges->check("ShipOrders"));
      }

      break;

    case 2:
      menuItem = pMenu->addAction("Issue Additional Stock to Order Line...", this, SLOT(sIssueStock()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));

      menuItem = pMenu->addAction("Return ALL Stock Issued to Order Line...", this, SLOT(sReturnAllLineStock()));
      menuItem->setEnabled(_privileges->check("ReturnStockFromShipping"));

      menuItem = pMenu->addAction("View Order Line...", this, SLOT(sViewLine()));
      menuItem->setEnabled(_privileges->check("ViewSalesOrders"));

      break;

    case 3:
      menuItem = pMenu->addAction("Return ALL of this Stock Issued in this Transaction...", this, SLOT(sReturnAllStock()));
      menuItem->setEnabled(_privileges->check("ReturnStockFromShipping"));

      break;
  }
}

void maintainShipping::sShippingInformation()
{
  ParameterList params;
  params.append("shiphead_id", _ship->id());
  
  shippingInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void maintainShipping::sShipOrder()
{
  ParameterList params;
  params.append("shiphead_id", _ship->id());
  
  shipOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void maintainShipping::sReturnAllOrderStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN");
  q.prepare("SELECT returnCompleteShipment(:ship_id) AS result;");
  q.bindValue(":ship_id", _ship->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("returnCompleteShipment", result),
		  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
      return;
    }
    q.exec("COMMIT;"); 
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sViewOrder()
{
  q.prepare( "SELECT shiphead_order_id, shiphead_order_type "
             "FROM shiphead "
             "WHERE (shiphead_id=:shiphead_id);" );
  q.bindValue(":shiphead_id", _ship->id());
  q.exec();
  if (q.first())
  {
    if (q.value("shiphead_order_type").toString() == "SO")
      salesOrder::viewSalesOrder(q.value("shiphead_order_id").toInt());
    else if (q.value("shiphead_order_type").toString() == "TO")
      transferOrder::viewTransferOrder(q.value("shiphead_order_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sPrintShippingForm()
{
  ParameterList params;
  params.append("shiphead_id", _ship->id());

  printShippingForm newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void maintainShipping::sIssueStock()
{
  ParameterList params;
  if (_ship->currentItem()->rawValue("ordertype").toString() == "SO")
	  params.append("sohead_id", _ship->id());
  else if (_ship->currentItem()->rawValue("ordertype").toString() == "TO")
    params.append("tohead_id", _ship->id());

  issueToShipping *newdlg = new issueToShipping();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void maintainShipping::sReturnAllLineStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN");
  q.prepare("SELECT returnItemShipments(:ship_id) AS result;");
  q.bindValue(":ship_id", _ship->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("returnItemShipments", result),
		  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
      return;
    }    
    q.exec("COMMIT;"); 
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sViewLine()
{
  ParameterList params;
  params.append("mode", "view");
  
  if (_ship->currentItem()->rawValue("ordertype").toString() == "SO")
  {
    params.append("soitem_id", _ship->altId());
    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }

  if (_ship->currentItem()->rawValue("ordertype").toString() == "TO")
  {
    params.append("toitem_id", _ship->altId());
    transferOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
}

void maintainShipping::sReturnAllStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN");
  q.prepare("SELECT returnShipmentTransaction(:ship_id) AS result;");
  q.bindValue(":ship_id", _ship->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError(this,
		  storedProcErrorLookup("returnShipmentTransaction", result),
		  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
      return;
    }    
    q.exec("COMMIT;"); 
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sFillList()
{
  ParameterList params;

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  params.append("notPrinted",	tr("No"));
  params.append("dirty",	tr("Dirty"));
  params.append("printed",	tr("Yes"));

  MetaSQLQuery mql = mqlLoad("maintainShipping", "detail");
  q = mql.toQuery(params);
  q.exec();
  _ship->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sPrint()
{
  ParameterList params;

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  params.append("notPrinted",	tr("No"));
  params.append("dirty",	tr("Dirty"));
  params.append("printed",	tr("Yes"));
  params.append("includeFormatted");

  orReport report("ShipmentsPending", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


