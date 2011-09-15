/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklog.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "parameterwidget.h"
#include "printPackingList.h"

dspBacklog::dspBacklog(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBacklog", fl)
{
  setWindowTitle(tr("Backlog"));
  setReportName("Backlog");
  setMetaSQLOptions("backlog", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Start Order Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Order Date"),   "endDate",   ParameterWidget::Date);
  parameterWidget()->append(tr("Start Schedule Date"), "startDateSched", ParameterWidget::Date);
  parameterWidget()->append(tr("End Schedule Date"),   "endDateSched",   ParameterWidget::Date);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Group"), "custgrp_id", XComboBox::CustomerGroups);
  parameterWidget()->append(tr("Customer Group Pattern"), "custgrp_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Customer Ship-To"), "shipto_id", ParameterWidget::Shipto);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Sales Order"), "cohead_id", ParameterWidget::SalesOrder);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesRepsActive);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  list()->addColumn(tr("S/O #/Line #"),_itemColumn, Qt::AlignLeft,  true, "coitem_linenumber");
  list()->addColumn(tr("Customer/Item Number"), -1, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Order"),       _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  list()->addColumn(tr("Ship/Sched."), _dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  list()->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Ordered"),      _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  list()->addColumn(tr("Shipped"),      _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  list()->addColumn(tr("Balance"),      _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    list()->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  list()->setPopulateLinear(true);
}

enum SetResponse dspBacklog::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custtype_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer Type"), param);

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer Type Pattern"), param);

  param = pParams.value("custgrp_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer Group"), param);

  param = pParams.value("custgrp_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer Group Pattern"), param);

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Product Category"), param);

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Product Category Pattern"), param);

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param);

  param = pParams.value("sohead_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Sales Order"), param);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspBacklog::sEditOrder()
{
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspBacklog::sViewOrder()
{
  salesOrder::viewSalesOrder(list()->id());
}

void dspBacklog::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklog::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklog::sPrintPackingList()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("sohead_id", ((XTreeWidgetItem*)(selected[i]))->id());

    printPackingList newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspBacklog::sAddToPackingListBatch()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
    q.bindValue(":sohead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void dspBacklog::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  if (list()->id() <= 0)
    return;

  QList<XTreeWidgetItem*> selected = list()->selectedItems();

  QAction *menuItem;

  if (selected.size() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
    if (!_privileges->check("MaintainSalesOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      menuItem->setEnabled(false);

    if (list()->altId() != -1)
    {
      pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Edit Item..."), this, SLOT(sEditItem()));
      if (!_privileges->check("MaintainSalesOrders"))
        menuItem->setEnabled(false);

      menuItem = pMenu->addAction(tr("View Item..."), this, SLOT(sViewItem()));
      if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
        menuItem->setEnabled(false);
    }
  }

  if (list()->id() > 0)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
    if (!_privileges->check("PrintPackingLists"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Add to Packing List Batch"), this, SLOT(sAddToPackingListBatch()));
    if (!_privileges->check("MaintainPackingListBatch"))
      menuItem->setEnabled(false);
  }
}

void dspBacklog::sFillList()
{
  display::sFillList();
  list()->expandAll();
}

