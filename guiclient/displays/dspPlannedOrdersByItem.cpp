/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPlannedOrdersByItem.h"

#include <QAction>
#include <QMenu>

#include "guiclient.h"
#include "deletePlannedOrder.h"
#include "dspRunningAvailability.h"
#include "plannedOrder.h"
#include "firmPlannedOrder.h"
#include "purchaseRequest.h"
#include "transferOrder.h"
#include "workOrder.h"

dspPlannedOrdersByItem::dspPlannedOrdersByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPlannedOrdersByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Planned Orders by Item"));
  setListLabel(tr("Planned Orders"));
  setReportName("PlannedOrdersByItem");
  setMetaSQLOptions("schedule", "plannedorders");
  setUseAltId(true);

  list()->addColumn(tr("Order #"),    _orderColumn, Qt::AlignCenter,true, "ordernum");
  list()->addColumn(tr("Type"),       _uomColumn,   Qt::AlignCenter,true, "ordtype");
  list()->addColumn(tr("Site"),       _whsColumn,   Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("From Site"),  _whsColumn,   Qt::AlignCenter,true, "supply_warehous_code");
  list()->addColumn(tr("Start Date"), _dateColumn,  Qt::AlignCenter,true, "planord_startdate");
  list()->addColumn(tr("Due Date"),   _dateColumn,  Qt::AlignCenter,true, "planord_duedate");
  list()->addColumn(tr("Qty"),        _qtyColumn,   Qt::AlignRight, true, "planord_qty");
  list()->addColumn(tr("Firm"),       _uomColumn,   Qt::AlignCenter,true, "planord_firm");
  list()->addColumn(tr("Comments"),   -1,           Qt::AlignLeft,  true, "comments");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspPlannedOrdersByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPlannedOrdersByItem::setParams(ParameterList &pParams)
{
  pParams.append("item_id", _item->id());
  _warehouse->appendValue(pParams);

  return true;
}

void dspPlannedOrdersByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
  menuItem->setEnabled(_privileges->check("CreatePlannedOrders"));

  if (pSelected->text(7) == "No")
  {
    menuItem = pMenu->addAction(tr("Firm Order..."), this, SLOT(sFirmOrder()));
    menuItem->setEnabled(_privileges->check("FirmPlannedOrders"));
  }
  else
  {
    menuItem = pMenu->addAction(tr("Soften Order..."), this, SLOT(sSoftenOrder()));
    menuItem->setEnabled(_privileges->check("SoftenPlannedOrders"));
  }

  menuItem = pMenu->addAction(tr("Release Order..."), this, SLOT(sReleaseOrder()));
  if ( (!_privileges->check("ReleasePlannedOrders")) ||
       ((pSelected->text(1) == "T/O") && (!_privileges->check("MaintainTransferOrders")) ) ||
       ((pSelected->text(1) == "W/O") && (!_privileges->check("MaintainWorkOrders")) ) ||
       ((pSelected->text(1) == "P/O") && (!_privileges->check("MaintainPurchaseRequests")) ) )
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Delete Order..."), this, SLOT(sDeleteOrder()));
  menuItem->setEnabled(_privileges->check("DeletePlannedOrders"));
}

void dspPlannedOrdersByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPlannedOrdersByItem::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", list()->id());

  firmPlannedOrder newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByItem::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=false "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", list()->id());
  q.exec();

  sFillList();
}

void dspPlannedOrdersByItem::sReleaseOrder()
{
  if (list()->currentItem()->text(1) == "W/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", list()->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (list()->currentItem()->text(1) == "P/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", list()->id());

    purchaseRequest newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if (list()->currentItem()->text(1) == "T/O")
  {
    ParameterList params;
    params.append("mode", "releaseTO");
    params.append("planord_id", list()->id());

    transferOrder *newdlg = new transferOrder();
    if(newdlg->set(params) == NoError)
      omfgThis->handleNewWindow(newdlg);
    else
      delete newdlg;
  }
  sFillList();
}

void dspPlannedOrdersByItem::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("planord_id", list()->id());

  plannedOrder newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspPlannedOrdersByItem::sDeleteOrder()
{
  ParameterList params;
  params.append("planord_id", list()->id());

  deletePlannedOrder newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

