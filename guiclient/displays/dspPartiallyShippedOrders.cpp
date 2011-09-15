/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPartiallyShippedOrders.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "guiclient.h"
#include "printPackingList.h"
#include "salesOrder.h"

#define AMOUNT_COL	7
#define AMOUNT_CURR_COL	8
#define BASEAMOUNT_COL	9

dspPartiallyShippedOrders::dspPartiallyShippedOrders(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPartiallyShippedOrders", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Partially Shipped Orders"));
  setListLabel(tr("Sales Orders"));
  setReportName("PartiallyShippedOrders");
  setMetaSQLOptions("partiallyShippedOrders", "detail");
  setUseAltId(true);

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Hold"),        0,           Qt::AlignCenter,true, "cohead_holdtype");
  list()->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignRight, true, "cohead_number");
  list()->addColumn(tr("Customer"),    -1,          Qt::AlignLeft,  true, "cust_name");
  list()->addColumn(tr("Hold Type"),   _dateColumn, Qt::AlignCenter,true, "f_holdtype");
  list()->addColumn(tr("Ordered"),     _dateColumn, Qt::AlignRight, true, "cohead_orderdate");
  list()->addColumn(tr("Scheduled"),   _dateColumn, Qt::AlignRight, true, "minscheddate");
  list()->addColumn(tr("Pack Date"),   _dateColumn, Qt::AlignRight, true, "cohead_packdate");
  list()->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "extprice");
  list()->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currAbbr");
  list()->addColumn(tr("Amount\n(%1)").arg(CurrDisplay::baseCurrAbbr()),
                                   _moneyColumn, Qt::AlignRight, true, "extprice_base");
  list()->setDragString("soheadid=");

  sHandlePrices(_showPrices->isChecked());

  if ( (!_privileges->check("ViewCustomerPrices")) && (!_privileges->check("MaintainCustomerPrices")) )
    _showPrices->setEnabled(false);
}

void dspPartiallyShippedOrders::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspPartiallyShippedOrders::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
  {
    list()->showColumn(AMOUNT_COL);
    if (!omfgThis->singleCurrency())
      list()->showColumn(AMOUNT_CURR_COL);
    if (!omfgThis->singleCurrency())
      list()->showColumn(BASEAMOUNT_COL);
  }
  else
  {
    list()->hideColumn(AMOUNT_COL);
    list()->hideColumn(AMOUNT_CURR_COL);
    list()->hideColumn(BASEAMOUNT_COL);
  }
}

bool dspPartiallyShippedOrders::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  if (_dates->allValid())
    _dates->appendValue(params);
  else
    return false;

  if(_showPrices->isChecked())
    params.append("showPrices");

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("pack",   tr("Pack"));
  params.append("return", tr("Return"));
  params.append("ship",   tr("Ship"));
  params.append("other",  tr("Other"));

  if (omfgThis->singleCurrency())
    params.append("singlecurrency");

  return true;
}

void dspPartiallyShippedOrders::sEditOrder()
{
  salesOrder::editSalesOrder(list()->altId(), false);
}

void dspPartiallyShippedOrders::sViewOrder()
{
  salesOrder::viewSalesOrder(list()->altId());
}

void dspPartiallyShippedOrders::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", list()->altId());

  printPackingList newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspPartiallyShippedOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                       _privileges->check("ViewSalesOrders"));

  pMenu->addSeparator();

  if ( (list()->currentItem()->text(0) != "P") && (list()->currentItem()->text(0) != "C") )
  {
    menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
    menuItem->setEnabled(_privileges->check("PrintPackingLists"));
  }
}

