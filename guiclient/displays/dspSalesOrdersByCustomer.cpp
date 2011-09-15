/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrdersByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByCustomer::dspSalesOrdersByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesOrdersByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales Order Lookup by Customer"));
  setListLabel(tr("Sales Orders"));
  setMetaSQLOptions("salesOrders", "detail");

  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date:"));

  list()->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight,  true,  "cohead_orderdate"  );
  list()->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight,  true,  "min_scheddate"  );
  list()->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter, true,  "order_status" );
  list()->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "cohead_shiptoname"   );
  list()->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "cohead_custponumber"   );

  _cust->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspSalesOrdersByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSalesOrdersByCustomer::sPopulatePo()
{
  _poNumber->clear();

  if ((_cust->isValid()) && (_dates->allValid()))
  {
    q.prepare( "SELECT MIN(cohead_id), cohead_custponumber "
               "FROM cohead "
               "WHERE ( (cohead_cust_id=:cust_id)"
               " AND (cohead_orderdate BETWEEN :startDate AND :endDate) ) "
               "GROUP BY cohead_custponumber "
               "ORDER BY cohead_custponumber;" );
    _dates->bindValue(q);
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    _poNumber->populate(q);
  }
}

void dspSalesOrdersByCustomer::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
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

void dspSalesOrdersByCustomer::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspSalesOrdersByCustomer::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::viewSalesOrder(list()->id());
}

void dspSalesOrdersByCustomer::sCreateRA()
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

void dspSalesOrdersByCustomer::sDspShipmentStatus()
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

void dspSalesOrdersByCustomer::sDspShipments()
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

bool dspSalesOrdersByCustomer::setParams(ParameterList &params)
{
  if ( !(( (_allPOs->isChecked()) ||
         ( (_selectedPO->isChecked()) && (_poNumber->currentIndex() != -1) ) ) &&
       (_dates->allValid())  ))
  {
    QMessageBox::warning(this, tr("Incomplete Options"),
      tr("One or more of the options are incomplete."));
    return false;
  }

  _dates->appendValue(params);
  params.append("noLines", tr("No Lines"));
  params.append("closed", tr("Closed"));
  params.append("open", tr("Open"));
  params.append("partial", tr("Partial"));
  params.append("cust_id", _cust->id());
  if (_selectedPO->isChecked())
    params.append("poNumber", _poNumber->currentText());

  return true;
}

bool dspSalesOrdersByCustomer::checkSitePrivs(int orderid)
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
