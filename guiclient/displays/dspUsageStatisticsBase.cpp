/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsBase.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsBase::dspUsageStatisticsBase(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Usage"));
  setMetaSQLOptions("usageStatistics", "detail");
  setUseAltId(true);
  _printing = false;

  list()->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight,  true,  "received"  );
  list()->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight,  true,  "issued"  );
  list()->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight,  true,  "sold"  );
  list()->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight,  true,  "scrap"  );
  list()->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight,  true,  "adjust"  );
  if (_metrics->boolean("MultiWhs"))
    list()->addColumn(tr("Transfers"), _qtyColumn,  Qt::AlignRight,  true,  "transfer"  );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);

}

void dspUsageStatisticsBase::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspUsageStatisticsBase::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspUsageStatisticsBase::setParams(ParameterList & params)
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if(_parameter->isVisibleTo(this))
    _parameter->appendValue(params);

  if(_itemGroup->isVisibleTo(this))
    params.append("item_id", _item->id());

  if(_printing)
    params.append("print");

  return true;
}

void dspUsageStatisticsBase::sPrint()
{
  _printing = true;
  display::sPrint();
  _printing = false;
}

void dspUsageStatisticsBase::sPreview()
{
  _printing = true;
  display::sPreview();
  _printing = false;
}

void dspUsageStatisticsBase::sViewAll()
{
  viewTransactions(QString::null);
}

void dspUsageStatisticsBase::sViewReceipt()
{
  viewTransactions("R");
}

void dspUsageStatisticsBase::sViewIssue()
{
  viewTransactions("I");
}

void dspUsageStatisticsBase::sViewSold()
{
  viewTransactions("S");
}

void dspUsageStatisticsBase::sViewScrap()
{
  viewTransactions("SC");
}

void dspUsageStatisticsBase::sViewAdjustment()
{
  viewTransactions("A");
}

void dspUsageStatisticsBase::sViewTransfer()
{
  viewTransactions("T");
}

void dspUsageStatisticsBase::viewTransactions(QString pType)
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (!pType.isNull())
    params.append("transtype", pType);

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatisticsBase::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("View All Transactions...", this, SLOT(sViewAll()));
  if (!_privileges->check("ViewInventoryHistory"))
    menuItem->setEnabled(false);

  switch (pColumn)
  {
    case 3:
      menuItem = pMenu->addAction("View Receipt Transactions...", this, SLOT(sViewReceipt()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 4:
      menuItem = pMenu->addAction("View Issue Transactions...", this, SLOT(sViewIssue()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 5:
      menuItem = pMenu->addAction("View Sold Transactions...", this, SLOT(sViewSold()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 6:
      menuItem = pMenu->addAction("View Scrap Transactions...", this, SLOT(sViewScrap()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 7:
      menuItem = pMenu->addAction("View Adjustment Transactions...", this, SLOT(sViewAdjustment()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 8:
      menuItem = pMenu->addAction("View Transfer Transactions...", this, SLOT(sViewTransfer()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;
  }
}

