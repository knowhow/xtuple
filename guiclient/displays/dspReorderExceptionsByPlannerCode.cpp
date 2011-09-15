/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspReorderExceptionsByPlannerCode.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "dspRunningAvailability.h"
#include "workOrder.h"

dspReorderExceptionsByPlannerCode::dspReorderExceptionsByPlannerCode(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspReorderExceptionsByPlannerCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Reorder Exceptions by Planner Code"));
  setListLabel(tr("Expedite Exceptions"));
  setReportName("ReorderExceptionsByPlannerCode");
  setMetaSQLOptions("reorderExceptionsByPlannerCode", "detail");
  setUseAltId(true);

  _plannerCode->setType(ParameterGroup::PlannerCode);

  list()->addColumn(tr("Site"),           _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),    -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Exception Date"), _dateColumn, Qt::AlignCenter, true,  "reorderdate" );
  list()->addColumn(tr("Reorder Level"),  _qtyColumn,  Qt::AlignRight,  true,  "reorderlevel"  );
  list()->addColumn(tr("Proj. Avail."),   _qtyColumn,  Qt::AlignRight,  true,  "projavail"  );
}

void dspReorderExceptionsByPlannerCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspReorderExceptionsByPlannerCode::setParams(ParameterList &params)
{
  params.append("lookAheadDays", _days->value());

  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_includePlanned->isChecked())
    params.append("includePlannedOrders");

  return true;
}

void dspReorderExceptionsByPlannerCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Running Availability..."), this, SLOT(sRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  if (list()->altId() == 1)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Create Work Order..."), this, SLOT(sCreateWorkOrder()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
  }
}

void dspReorderExceptionsByPlannerCode::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspReorderExceptionsByPlannerCode::sCreateWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
