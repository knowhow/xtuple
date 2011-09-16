/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogBySalesOrder.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "dspRunningAvailability.h"

dspBacklogBySalesOrder::dspBacklogBySalesOrder(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBacklogBySalesOrder", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Backlog by Sales Order"));
  setListLabel(tr("Shipment Status"));
  setReportName("BacklogBySalesOrder");
  setMetaSQLOptions("salesOrderItems", "detail");
  setUseAltId(true);

  _salesOrder->setAllowedTypes(OrderLineEdit::Sales);
  _salesOrder->setAllowedStatuses(OrderLineEdit::Open);

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _salesOrder, SLOT(setId(int)));

  list()->addColumn(tr("#"),           _seqColumn,  Qt::AlignCenter,true, "coitem_linenumber");
  list()->addColumn(tr("Item"),        _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "itemdescription");
  list()->addColumn(tr("Site"),        _whsColumn,  Qt::AlignLeft,  true, "warehous_code");
  list()->addColumn(tr("Ordered"),     _qtyColumn,  Qt::AlignRight, true, "coitem_qtyord");
  list()->addColumn(tr("Shipped"),     _qtyColumn,  Qt::AlignRight, true, "coitem_qtyshipped");
  list()->addColumn(tr("Balance"),     _qtyColumn,  Qt::AlignRight, true, "qtybalance");
  list()->addColumn(tr("At Shipping"), _qtyColumn,  Qt::AlignRight, true, "qtyatshipping");
  list()->addColumn(tr("Available"),   _qtyColumn,  Qt::AlignRight, true, "qtyavailable");
}

void dspBacklogBySalesOrder::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspBacklogBySalesOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  if (list()->id() <= 0)
    return;

  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));
}

bool dspBacklogBySalesOrder::setParams(ParameterList &params)
{
  params.append("sohead_id", _salesOrder->id()); // report
  params.append("cohead_id", _salesOrder->id()); // metasql

  return true;
}

void dspBacklogBySalesOrder::sRunningAvailability()
{
  q.prepare( "SELECT coitem_itemsite_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", list()->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("coitem_itemsite_id").toInt());
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspBacklogBySalesOrder::sFillList()
{
  if (_salesOrder->isValid())
  {
    q.prepare( "SELECT cohead_number,"
               "       cohead_orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", _salesOrder->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("cohead_orderdate").toDate());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }

    display::sFillList();
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    list()->clear();
  }
}
