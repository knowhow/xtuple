/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrdersByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByItem::dspSalesOrdersByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesOrdersByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales Order Lookup by Item"));
  setListLabel(tr("Sales Orders"));
  setMetaSQLOptions("salesOrderItems", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _item->setType(ItemLineEdit::cSold);

  list()->addColumn(tr("Order #"),    _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Order Date"), _dateColumn,  Qt::AlignCenter, true,  "cohead_orderdate" );
  list()->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Ordered"),    _qtyColumn,   Qt::AlignRight,  true,  "coitem_qtyord"  );
  list()->addColumn(tr("Shipped"),    _qtyColumn,   Qt::AlignRight,  true,  "coitem_qtyshipped"  );
  list()->addColumn(tr("Returned"),   _qtyColumn,   Qt::AlignRight,  true,  "coitem_qtyreturned"  );
  list()->addColumn(tr("Balance"),    _qtyColumn,   Qt::AlignRight,  true,  "qtybalance"  );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspSalesOrdersByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSalesOrdersByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  return NoError;
}

void dspSalesOrdersByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  if(_privileges->check("MaintainSalesOrders"))
    menuThis->addAction(tr("Edit..."), this, SLOT(sEditOrder()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewOrder()));
  menuThis->addSeparator();
  menuThis->addAction(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()));
  menuThis->addAction(tr("Shipments.."), this, SLOT(sDspShipments()));

  if ( (_metrics->boolean("EnableReturnAuth")) && (_privileges->check("MaintainReturns")) )
  {
    menuThis->addSeparator();
    menuThis->addAction(tr("Create Return Authorization..."), this, SLOT(sCreateRA()));
  }
}

void dspSalesOrdersByItem::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspSalesOrdersByItem::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  salesOrder::viewSalesOrder(list()->id());
}

void dspSalesOrdersByItem::sCreateRA()
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

void dspSalesOrdersByItem::sDspShipmentStatus()
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

void dspSalesOrdersByItem::sDspShipments()
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

bool dspSalesOrdersByItem::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Dates Required"),
      tr("You must specify a valid Date range."));
    return false;
  }

  _dates->appendValue(params);
  params.append("closed", tr("Closed"));
  params.append("item_id", _item->id());

  return true;
}

bool dspSalesOrdersByItem::checkSitePrivs(int orderid)
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
                              tr("<p>You may not view or edit this Sales Order "
                                 "as it references a Site for which you have "
                                 "not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
