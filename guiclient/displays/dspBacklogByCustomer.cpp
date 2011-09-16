/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByCustomer::dspBacklogByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBacklogByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Backlog by Customer"));
  setListLabel(tr("Backlog"));
  setReportName("BacklogByCustomer");
  setMetaSQLOptions("salesOrderItems", "detail");
  setUseAltId(true);

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));

  _cust->setFocus();

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  list()->addColumn(tr("S/O #/Line #"), _itemColumn, Qt::AlignLeft,  true, "coitem_linenumber");
  list()->addColumn(tr("Customer/Item Number"),  -1, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Order"),        _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  list()->addColumn(tr("Ship/Sched."),  _dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  list()->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Ordered"),       _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  list()->addColumn(tr("Shipped"),       _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  list()->addColumn(tr("Balance"),       _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    list()->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    list()->hideColumn("baseextpricebalance");

  _cust->setFocus();
}

void dspBacklogByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspBacklogByCustomer::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    list()->showColumn("baseextpricebalance");
  else
    list()->hideColumn("baseextpricebalance");

  if (_cust->isValid())
  {
    sFillList();
  }
}

bool dspBacklogByCustomer::setParams(ParameterList &params)
{
  if (!_cust->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Customer Number"),
                         tr("<p>You must enter a valid Customer Number for "
                            "this report.") );
    _cust->setFocus();
    return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  params.append("cust_id", _cust->id());
  params.append("openOnly");
  params.append("orderByScheddate");

  if (_showPrices->isChecked())
    params.append("showPrices");

  return true;
}

void dspBacklogByCustomer::sEditOrder()
{
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspBacklogByCustomer::sViewOrder()
{
  salesOrder::viewSalesOrder(list()->id());
}

void dspBacklogByCustomer::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByCustomer::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByCustomer::sPrintPackingList()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(selected[i]);
    if (cursor->altId() == -1)
    {
      ParameterList params;
      params.append("sohead_id", cursor->id());

      printPackingList newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
  }
}

void dspBacklogByCustomer::sAddToPackingListBatch()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(selected[i]);
    if (cursor->altId() == -1)
    {
      q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
      q.bindValue(":sohead_id", cursor->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
}

void dspBacklogByCustomer::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  if (list()->id() <= 0)
    return;

  bool hasParents     = false;
  bool hasChildren    = false;
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(selected[i]);
    if ( (cursor->altId() == -1) && (!hasParents) )
      hasParents = true;

    if ( (cursor->altId() != -1) && (!hasChildren) )
      hasChildren = true;
  }

  QAction *menuItem;

  if (selected.size() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
    if (!_privileges->check("MaintainSalesOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      menuItem->setEnabled(false);

    if (hasChildren)
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

  if (hasParents)
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

