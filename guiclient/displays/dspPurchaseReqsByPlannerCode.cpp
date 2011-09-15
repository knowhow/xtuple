/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPurchaseReqsByPlannerCode.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "xtreewidget.h"

#include "dspRunningAvailability.h"
#include "purchaseOrder.h"

dspPurchaseReqsByPlannerCode::dspPurchaseReqsByPlannerCode(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPurchaseReqsByPlannerCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Requests by Planner Code"));
  setListLabel(tr("Purchase Requests"));
  setReportName("PurchaseReqsByPlannerCode");
  setMetaSQLOptions("purchase", "purchaserequests");
  setUseAltId(true);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  _plannerCode->setType(ParameterGroup::PlannerCode);

  list()->addColumn(tr("P/R #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_number");
  list()->addColumn(tr("Sub #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_subnumber");
  list()->addColumn(tr("Item Number"),  _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),  -1,            Qt::AlignLeft,   true,  "description"   );
  list()->addColumn(tr("Status"),       _statusColumn, Qt::AlignCenter, true,  "pr_status" );
  list()->addColumn(tr("Parent Order"), _itemColumn,   Qt::AlignLeft,   true,  "parent"   );
  list()->addColumn(tr("Create Date"),        -1,    Qt::AlignLeft,  true,  "pr_createdate"  );
  list()->addColumn(tr("Due Date"),     _dateColumn,   Qt::AlignCenter, true,  "pr_duedate" );
  list()->addColumn(tr("Qty."),         _qtyColumn,    Qt::AlignRight,  true,  "pr_qtyreq"  );
  list()->addColumn(tr("Notes"),        -1,    Qt::AlignLeft,  true,  "pr_releasenote"  );
}

void dspPurchaseReqsByPlannerCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPurchaseReqsByPlannerCode::setParams(ParameterList &params)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return false;
  }

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  params.append("manual", tr("Manual"));
  params.append("other",  tr("Other"));

  return true;
}

void dspPurchaseReqsByPlannerCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Release P/R..."), this, SLOT(sRelease()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Delete P/R..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));
}

void dspPurchaseReqsByPlannerCode::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPurchaseReqsByPlannerCode::sRelease()
{
  ParameterList params;
  params.append("mode", "releasePr");
  params.append("pr_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    delete newdlg;

  sFillList();
  omfgThis->sPurchaseRequestsUpdated();
}

void dspPurchaseReqsByPlannerCode::sDelete()
{
  q.prepare("SELECT deletePr(:pr_id) AS _result;");
  q.bindValue(":pr_id", list()->id());
  q.exec();

  sFillList();
  omfgThis->sPurchaseRequestsUpdated();
}

