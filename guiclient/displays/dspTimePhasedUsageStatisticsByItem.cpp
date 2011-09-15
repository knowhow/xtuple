/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedUsageStatisticsByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspInventoryHistory.h"
#include "guiclient.h"

dspTimePhasedUsageStatisticsByItem::dspTimePhasedUsageStatisticsByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedUsageStatisticsByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Item Usage Statistics by Item"));
  setListLabel(tr("Usage"));
  setReportName("TimePhasedStatisticsByItem");
  setMetaSQLOptions("timePhasedUsageStatisticsByItem", "detail");

  list()->addColumn(tr("Transaction Type"), 120,        Qt::AlignLeft,   true, "label");
  list()->addColumn(tr("Site"),             _whsColumn, Qt::AlignCenter, true, "warehous_code" );
}

void dspTimePhasedUsageStatisticsByItem::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedUsageStatisticsByItem::sViewTransactions()
{
  if (_column > 1)
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("startDate",   _columnDates[_column - 2].startDate);
    params.append("endDate",     _columnDates[_column - 2].endDate);
    params.append("run");

    QString type = list()->currentItem()->text(0);
    if (type == "Received")
      params.append("transtype", "R");
    else if (type == "Issued")
      params.append("transtype", "I");
    else if (type == "Sold")
      params.append("transtype", "S");
    else if (type == "Scrap")
      params.append("transtype", "SC");
    else if (type == "Adjustments")
      params.append("transtype", "A");

    dspInventoryHistory *newdlg = new dspInventoryHistory();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedUsageStatisticsByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  menuItem = menu->addAction(tr("View Transactions..."), this, SLOT(sViewTransactions()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
}

bool dspTimePhasedUsageStatisticsByItem::setParamsTP(ParameterList & params)
{
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  params.append("received", tr("Received"));
  params.append("issued", tr("Issued"));
  params.append("sold", tr("Sold"));
  params.append("scrap", tr("Scrap"));
  params.append("adjustments", tr("Adjustments"));

  return true;
}

