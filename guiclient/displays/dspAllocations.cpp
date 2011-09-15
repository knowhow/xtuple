/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAllocations.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "salesOrder.h"
#include "transferOrder.h"
#include "workOrder.h"

dspAllocations::dspAllocations(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspAllocations", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Allocations"));
  setListLabel(tr("Item Allocations"));
  setMetaSQLOptions("allocations", "detail");

  _item->setReadOnly(true);
  _warehouse->setEnabled(false);

  list()->addColumn(tr("Type"),         _docTypeColumn, Qt::AlignCenter,true, "type");
  list()->addColumn(tr("Order #"),      _orderColumn,   Qt::AlignLeft,  true, "order_number");
  list()->addColumn(tr("Parent Item"),  -1,             Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Total Qty."),   _qtyColumn,     Qt::AlignRight, true, "totalqty");
  list()->addColumn(tr("Relieved"),     _qtyColumn,     Qt::AlignRight, true, "relievedqty");
  list()->addColumn(tr("Balance"),      _qtyColumn,     Qt::AlignRight, true, "balanceqty");
  list()->addColumn(tr("Running Bal."), _qtyColumn,     Qt::AlignRight, true, "runningbal");
  list()->addColumn(tr("Required"),     _dateColumn,    Qt::AlignCenter,true, "duedate");

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

void dspAllocations::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspAllocations::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDate", &valid);
  if (valid)
  {
    _byDate->setChecked(true);
    _date->setDate(param.toDate());
  }

  param = pParams.value("byDays", &valid);
  if (valid)
  {
    _byDays->setChecked(true);
    _days->setValue(param.toInt());
  }

  _byRange->setChecked(pParams.inList("byRange"));

  param = pParams.value("startDate", &valid);
  if (valid)
    _startDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endDate->setDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAllocations::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  if (QString(pSelected->text(0)) == "W/O")
  {
    menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sViewWorkOrder()));
    menuItem->setEnabled(_privileges->check("ViewWorkOrders"));
  }
  else if (QString(pSelected->text(0)) == "S/O")
  {
    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewCustomerOrder()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));

    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEditCustomerOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));
  }
  else if (QString(pSelected->text(0)) == "T/O")
  {
    menuItem = pMenu->addAction(tr("View Transfer Order..."), this, SLOT(sViewTransferOrder()));
    menuItem->setEnabled(_privileges->check("ViewTransferOrders"));

    menuItem = pMenu->addAction(tr("Edit Transfer Order..."), this, SLOT(sEditTransferOrder()));
    menuItem->setEnabled(_privileges->check("MaintainTransferOrders"));
  }
}

void dspAllocations::sViewWorkOrder()
{
  q.prepare( "SELECT womatl_wo_id "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("womatl_wo_id").toInt());
  
    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAllocations::sViewCustomerOrder()
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", list()->id());
  q.exec();
  if (q.first())
    salesOrder::viewSalesOrder(q.value("coitem_cohead_id").toInt());
}

void dspAllocations::sEditCustomerOrder()
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", list()->id());
  q.exec();
  if (q.first())
    salesOrder::editSalesOrder(q.value("coitem_cohead_id").toInt(), false);
}

void dspAllocations::sViewTransferOrder()
{
  q.prepare( "SELECT toitem_tohead_id "
             "FROM toitem "
             "WHERE (toitem_id=:toitem_id);" );
  q.bindValue(":toitem_id", list()->id());
  q.exec();
  if (q.first())
    transferOrder::viewTransferOrder(q.value("toitem_tohead_id").toInt());
}

void dspAllocations::sEditTransferOrder()
{
  q.prepare( "SELECT toitem_tohead_id "
             "FROM toitem "
             "WHERE (toitem_id=:toitem_id);" );
  q.bindValue(":toitem_id", list()->id());
  q.exec();
  if (q.first())
    transferOrder::editTransferOrder(q.value("toitem_tohead_id").toInt(), false);
}

bool dspAllocations::setParams(ParameterList &params)
{
  if (!( (_item->isValid()) &&
       ( (_leadTime->isChecked()) || (_byDays->isChecked()) ||
         ((_byDate->isChecked()) && (_date->isValid())) ||
         ((_byRange->isChecked()) && (_startDate->isValid()) && (_endDate->isValid())) ) ))
    return false;

  params.append("warehous_id", _warehouse->id());
  params.append("item_id",	   _item->id());
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_leadTime->isChecked())
    params.append("leadTime");
  else if (_byDays->isChecked())
    params.append("days", _days->value());
  else if (_byDate->isChecked())
    params.append("date", _date->date());
  else if (_byRange->isChecked())
  {
    params.append("startDate", _startDate->date());
    params.append("endDate",   _endDate->date());
  }

  return true;
}
