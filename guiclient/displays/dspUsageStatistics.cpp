/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatistics.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "dspInventoryHistory.h"
#include "parameterwidget.h"
#include "xtreewidget.h"

dspUsageStatistics::dspUsageStatistics(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, name, fl)
{
  setWindowTitle(tr("Item Usage Statistics"));
  setReportName("UsageStatistics");
  setMetaSQLOptions("usageStatistics", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  _printing = false;

  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate(QDate::currentDate().year(),1,1), true);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate(), true);
  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->append(tr("Item Group Pattern"), "itemgrp_pattern", ParameterWidget::Text);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

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
}

enum SetResponse dspUsageStatistics::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  parameterWidget()->setSavedFilters();

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param);

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    if (param.toInt() > 0)
      parameterWidget()->setDefault(tr("Site"), param);
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Start Date"), param);
  else
    parameterWidget()->setDefault(tr("Start Date"), omfgThis->startOfTime());

  param = pParams.value("endDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("End Date"), param);
  else
    parameterWidget()->setDefault(tr("End Date"), omfgThis->endOfTime());

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspUsageStatistics::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if(_printing)
    params.append("print");

  return true;
}

void dspUsageStatistics::sPrint()
{
  _printing = true;
  display::sPrint();
  _printing = false;
}

void dspUsageStatistics::sPreview()
{
  _printing = true;
  display::sPreview();
  _printing = false;
}

void dspUsageStatistics::sViewAll()
{
  viewTransactions(QString::null);
}

void dspUsageStatistics::sViewReceipt()
{
  viewTransactions("R");
}

void dspUsageStatistics::sViewIssue()
{
  viewTransactions("I");
}

void dspUsageStatistics::sViewSold()
{
  viewTransactions("S");
}

void dspUsageStatistics::sViewScrap()
{
  viewTransactions("SC");
}

void dspUsageStatistics::sViewAdjustment()
{
  viewTransactions("A");
}

void dspUsageStatistics::sViewTransfer()
{
  viewTransactions("T");
}

void dspUsageStatistics::viewTransactions(QString pType)
{
  ParameterList params;
  display::setParams(params);
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (!pType.isNull())
    params.append("transtype", pType);

  dspInventoryHistory *newdlg = new dspInventoryHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatistics::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
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

