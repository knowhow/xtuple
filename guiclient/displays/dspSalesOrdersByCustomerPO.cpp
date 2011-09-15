/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrdersByCustomerPO.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByCustomerPO::dspSalesOrdersByCustomerPO(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesOrdersByCustomerPO", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales Order Lookup by Customer PO"));
  setListLabel(tr("Sales Orders"));
  setMetaSQLOptions("salesOrders", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date:"));

  list()->addColumn(tr("Cust #"),      _itemColumn,  Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Customer"),    _itemColumn,  Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight,  true,  "cohead_orderdate"  );
  list()->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight,  true,  "min_scheddate"  );
  list()->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter, true,  "order_status" );
  list()->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "cohead_shiptoname"   );
  list()->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "cohead_custponumber"   );

  _poNumber->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspSalesOrdersByCustomerPO::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSalesOrdersByCustomerPO::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  if(_privileges->check("MaintainSalesOrders"))
    menuThis->addAction(tr("Edit..."), this, SLOT(sEditOrder()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewOrder()));
  menuThis->addSeparator();
  menuThis->addAction(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()));
  menuThis->addAction(tr("Shipments..."), this, SLOT(sDspShipments()));

  if ( (_metrics->boolean("EnableReturnAuth")) && (_privileges->check("MaintainReturns")) )
  {
    menuThis->addSeparator();
    menuThis->addAction(tr("Create Return Authorization..."), this, SLOT(sCreateRA()));
  }
}

void dspSalesOrdersByCustomerPO::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspSalesOrdersByCustomerPO::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::viewSalesOrder(list()->id());
}

void dspSalesOrdersByCustomerPO::sCreateRA()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("mode", "new");
  params.append("sohead_id", list()->id());

  returnAuthorization *newdlg = new returnAuthorization();
  if (newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
			  tr("The new Return Authorization could not be created"));
}

void dspSalesOrdersByCustomerPO::sDspShipmentStatus()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrdersByCustomerPO::sDspShipments()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", list()->id());
  params.append("run");

  dspShipmentsBySalesOrder *newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspSalesOrdersByCustomerPO::setParams(ParameterList &params)
{
  if (_poNumber->text().trimmed().length() == 0)    
  {
    QMessageBox::warning(this, tr("P/O Number Required"),
      tr("You must specify a P/O Number."));
    return false;
  }

  if (!_dates->allValid()) 
  {
    QMessageBox::warning(this, tr("Dates Required"),
      tr("You must specify a valid Date Range."));
    return false;
  }

  _dates->appendValue(params);
  params.append("noLines", tr("No Lines"));
  params.append("closed", tr("Closed"));
  params.append("open", tr("Open"));
  params.append("partial", tr("Partial"));
  params.append("poNumber", _poNumber->text());

  return true;
}

bool dspSalesOrdersByCustomerPO::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Sales Order as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

