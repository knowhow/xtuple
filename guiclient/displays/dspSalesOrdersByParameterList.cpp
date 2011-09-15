/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrdersByParameterList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByParameterList::dspSalesOrdersByParameterList(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesOrdersByParameterList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales Order Lookup by Parameter List"));
  setListLabel(tr("Sales Orders"));
  setMetaSQLOptions("salesOrders", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date:"));

  list()->addColumn(tr("Customer"),    _itemColumn,  Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight,  true,  "cohead_orderdate"  );
  list()->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight,  true,  "min_scheddate"  );
  list()->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter, true,  "order_status" );
  list()->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "cohead_shiptoname"   );
  list()->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "cohead_custponumber"   );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspSalesOrdersByParameterList::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSalesOrdersByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custtype", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CustomerType);

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerType);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerType);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (_parameter->type() == ParameterGroup::CustomerType)
    setWindowTitle(tr("Sales Order Lookup by Customer Type"));

  return NoError;
}

void dspSalesOrdersByParameterList::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  if(list()->id() == -1)
    return;

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

void dspSalesOrdersByParameterList::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspSalesOrdersByParameterList::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::viewSalesOrder(list()->id());
}

void dspSalesOrdersByParameterList::sCreateRA()
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

void dspSalesOrdersByParameterList::sDspShipmentStatus()
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

void dspSalesOrdersByParameterList::sDspShipments()
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

bool dspSalesOrdersByParameterList::setParams(ParameterList &params)
{
  if (!_dates->allValid()) 
  {
    QMessageBox::warning(this, tr("Dates Required"),
      tr("You must specify a valid Date range."));
    return false;
  }

  _dates->appendValue(params);
  _parameter->appendValue(params);
  params.append("noLines", tr("No Lines"));
  params.append("closed", tr("Closed"));
  params.append("open", tr("Open"));
  params.append("partial", tr("Partial"));
  params.append("orderByCust");

  return true;
}

bool dspSalesOrdersByParameterList::checkSitePrivs(int orderid)
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

