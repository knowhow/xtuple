/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByItem::dspBacklogByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBacklogByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Backlog by Item"));
  setListLabel(tr("Backlog"));
  setReportName("BacklogByItemNumber");
  setMetaSQLOptions("salesOrderItems", "detail");
  setUseAltId(true);

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));

  _item->setType(ItemLineEdit::cSold);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("S/O #"),   _orderColumn, Qt::AlignLeft,  true, "cohead_number");
  list()->addColumn(tr("#"),         _seqColumn, Qt::AlignCenter,true, "coitem_linenumber");
  list()->addColumn(tr("Customer"),          -1, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Ordered"),  _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  list()->addColumn(tr("Scheduled"),_dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  list()->addColumn(tr("Qty. UOM"),  _qtyColumn, Qt::AlignRight, true, "uom_name");
  list()->addColumn(tr("Ordered"),   _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  list()->addColumn(tr("Shipped"),   _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  list()->addColumn(tr("Balance"),   _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    list()->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    list()->hideColumn("baseextpricebalance");

  _item->setFocus();
}

void dspBacklogByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspBacklogByItem::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    list()->showColumn("baseextpricebalance");
  else
    list()->hideColumn("baseextpricebalance");

  if (_item->isValid())
    sFillList();
}

bool dspBacklogByItem::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Item Number"),
                         tr("<p>You must enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("openOnly");
  params.append("orderByScheddate");

  if (_showPrices->isChecked())
    params.append("showPrices");

  return true;
}

void dspBacklogByItem::sEditOrder()
{
  salesOrder::editSalesOrder(list()->id(), false);
}

void dspBacklogByItem::sViewOrder()
{
  salesOrder::viewSalesOrder(list()->id());
}

void dspBacklogByItem::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", list()->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", list()->id());

  printPackingList newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  if (list()->id() <= 0)
    return;
	
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
  if (!_privileges->check("MaintainSalesOrders"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
  if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
    menuItem->setEnabled(false);


  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit Item..."), this, SLOT(sEditItem()));
  if (!_privileges->check("MaintainSalesOrders"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Item..."), this, SLOT(sViewItem()));
  if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
  if (!_privileges->check("PrintPackingLists"))
    menuItem->setEnabled(false);
}

