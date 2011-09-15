/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedAvailability.h"

#include <QVariant>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <datecluster.h>

#include "guiclient.h"
#include "dspInventoryAvailability.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"

dspTimePhasedAvailability::dspTimePhasedAvailability(QWidget* parent, const char*, Qt::WFlags fl)
    : displayTimePhased(parent, "dspTimePhasedAvailability", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Availability"));
  setListLabel(tr("Time-Phased Availability"));
  setReportName("TimePhasedAvailability");
  setMetaSQLOptions("timePhasedAvailability", "detail");
  setUseAltId(true);

  _plannerCode->setType(ParameterGroup::PlannerCode);

  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "uom_name"   );
  list()->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
}

void dspTimePhasedAvailability::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedAvailability::sViewAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("byDate", _columnDates[_column - 3].startDate);
  params.append("run");

  dspInventoryAvailability *newdlg = new dspInventoryAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("byRange");
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("run");

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("byRange");
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("run");

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspTimePhasedAvailability::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;
  if (_column > 2)
  {
    menuItem = pMenu->addAction(tr("View Availability Detail..."), this, SLOT(sViewAvailability()));
    menuItem = pMenu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
    menuItem = pMenu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  
    if (((XTreeWidgetItem *)pSelected)->altId() == 1)
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("Create W/O..."), this, SLOT(sCreateWO()));
    }
    else if (((XTreeWidgetItem *)pSelected)->altId() == 2)
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
      menuItem = pMenu->addAction(tr("Create P/O..."), this, SLOT(sCreatePO()));
    }
  }
}

bool dspTimePhasedAvailability::setParamsTP(ParameterList & params)
{
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  return true;
}

