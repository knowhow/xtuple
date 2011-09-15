/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoHistoryByClassCode.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "workOrder.h"

dspWoHistoryByClassCode::dspWoHistoryByClassCode(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoHistoryByClassCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Work Order History by Class Code"));
  setListLabel(tr("Work Orders"));
  setReportName("WOHistoryByClassCode");
  setMetaSQLOptions("workOrderHistory", "detail");

  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _classCode->setType(ParameterGroup::ClassCode);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft,   true,  "wonumber"   );
  list()->addColumn(tr("Item #"),      _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,            Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  list()->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  list()->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  list()->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignRight,  true,  "wo_startdate"  );
  list()->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignRight,  true,  "wo_duedate"  );
  list()->addColumn(tr("Cost"),        _costColumn,   Qt::AlignRight,  true,  "wo_postedvalue" );

  sHandleCosts(_showCost->isChecked());
}

void dspWoHistoryByClassCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspWoHistoryByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByClassCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void dspWoHistoryByClassCode::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    list()->showColumn(9);
  else
    list()->hideColumn(9);
}

bool dspWoHistoryByClassCode::setParams(ParameterList & params)
{
  if(isVisible())
  {
    if (! _dates->startDate().isValid())
    {
      QMessageBox::warning(this, tr("Invalid Start Date"),
                           tr("Enter a valid Start Date."));
      _dates->setFocus();
      return false;
    }

    if (! _dates->endDate().isValid())
    {
      QMessageBox::warning(this, tr("Invalid End Date"),
                           tr("Enter a valid End Date."));
      _dates->setFocus();
      return false;
    }
  }

  params.append("woHistoryByClassCode"); // metasql only?

  _classCode->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_topLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts"); // report only?

  return true;
}


