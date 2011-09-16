/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogByParameterList.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByParameterList::dspBacklogByParameterList(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBacklogByParameterList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Backlog by Parameter List"));
  setListLabel(tr("Backlog"));
  setReportName("BacklogByParameterList");
  setMetaSQLOptions("salesOrderItems", "detail");
  setUseAltId(true);

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

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

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    list()->hideColumn("baseextpricebalance");
}

void dspBacklogByParameterList::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspBacklogByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

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

  param = pParams.value("custtype", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CustomerType);

  param = pParams.value("custgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CustomerGroup);

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ProductCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ProductCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("prodcat", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ProductCategory);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  switch (_parameter->type())
  {
    case ParameterGroup::CustomerType:
      setWindowTitle(tr("Backlog by Customer Type"));
      break;

    case ParameterGroup::CustomerGroup:
      setWindowTitle(tr("Backlog by Customer Group"));
      break;

    case ParameterGroup::ProductCategory:
      setWindowTitle(tr("Backlog by Product Category"));
      break;

    default:
      break;
  }

  return NoError;
}

void dspBacklogByParameterList::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    list()->showColumn("baseextpricebalance");
  else
    list()->hideColumn("baseextpricebalance");
}

bool dspBacklogByParameterList::setParams(ParameterList &params)
{
  if (! _dates->allValid())
  {
    _dates->setFocus();
    return false;
  }

  params.append("openOnly");
  params.append("orderByScheddate");

  _warehouse->appendValue(params);
  _parameter->appendValue(params);
  _dates->appendValue(params);

  if (_showPrices->isChecked())
    params.append("showPrices");

  if (_parameter->isAll())
  {
    switch (_parameter->type())
    {
      case ParameterGroup::CustomerType:
        params.append("custtype");
        break;

      case ParameterGroup::CustomerGroup:
        params.append("custgrp");
        break;

      case ParameterGroup::ProductCategory:
        params.append("prodcat");
        break;

      default:
        break;
    }
  }

  return true;
}

void dspBacklogByParameterList::sEditOrder()
{
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspBacklogByParameterList::sViewOrder()
{
  salesOrder::viewSalesOrder(list()->id());
}

void dspBacklogByParameterList::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByParameterList::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByParameterList::sPrintPackingList()
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

void dspBacklogByParameterList::sAddToPackingListBatch()
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

void dspBacklogByParameterList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
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

