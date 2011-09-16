/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPlannedOrders.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>

#include "guiclient.h"
#include "deletePlannedOrder.h"
#include "dspRunningAvailability.h"
#include "dspUsageStatistics.h"
#include "plannedOrder.h"
#include "firmPlannedOrder.h"
#include "purchaseRequest.h"
#include "transferOrder.h"
#include "workOrder.h"
#include "parameterwidget.h"

dspPlannedOrders::dspPlannedOrders(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspPlannedOrders", fl)
{
  setWindowTitle(tr("Planned Orders"));
  setReportName("PlannedOrders");
  setMetaSQLOptions("schedule", "plannedorders");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  QString qryType = QString( "SELECT  'P', '%1' UNION "
                             "SELECT  'W', '%2'")
      .arg(tr("Purchase"))
      .arg(tr("Manufacture"));

  if (_metrics->boolean("MultiWhs"))
    qryType.append(QString( " UNION "
                            "SELECT  'T', '%1'")
                   .arg(tr("Transfers")));

  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->append(tr("Order Types"), "type_list", ParameterWidget::Multiselect, QVariant(), false, qryType );
  parameterWidget()->appendComboBox(tr("Planner Code"), "plancode_id", XComboBox::PlannerCodes);
  parameterWidget()->append(tr("Planner Code Pattern"), "plancode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  list()->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,  true, "ordernum");
  list()->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter,true, "ordtype");
  list()->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("From Site"),   _whsColumn,   Qt::AlignCenter,true, "supply_warehous_code");
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "item_descrip");
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Start Date"),  _dateColumn,  Qt::AlignCenter,true, "planord_startdate");
  list()->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "planord_duedate");
  list()->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight, true, "planord_qty");
  list()->addColumn(tr("Firm"),        _ynColumn,    Qt::AlignCenter,true, "planord_firm");
}

enum SetResponse dspPlannedOrders::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  parameterWidget()->setSavedFilters();

  param = pParams.value("plancode_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Planner Code"), param);

  param = pParams.value("warehous_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Site"), param);

  param = pParams.value("type", &valid);
  if (valid)
  {
    QVariantList list;
    list.append(param.toString());
    parameterWidget()->setDefault(tr("Order Types"), list);
  }

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspPlannedOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));
  menuItem = pMenu->addAction(tr("Usage Statistics..."), this, SLOT(sDspUsageStatistics()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));

  pMenu->addSeparator();

  if (pSelected->text(10) == "No")
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("CreatePlannedOrders"));

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

void dspPlannedOrders::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPlannedOrders::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("planord_id", list()->id());

  plannedOrder newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspPlannedOrders::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", list()->id());

  firmPlannedOrder newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPlannedOrders::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=false "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", list()->id());
  q.exec();

  sFillList();
}

void dspPlannedOrders::sReleaseOrder()
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

    if (newdlg.exec() == XDialog::Rejected)
      return;
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
  QTreeWidgetItem * ci = list()->currentItem();
  if(ci)
    delete ci;
}

void dspPlannedOrders::sDeleteOrder()
{
  ParameterList params;
  params.append("planord_id", list()->id());

  deletePlannedOrder newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Rejected)
    return;
  QTreeWidgetItem * ci = list()->currentItem();
  if(ci)
    delete ci;
}

void dspPlannedOrders::sDspUsageStatistics()
{
  q.prepare("SELECT itemsite_item_id "
	    "FROM itemsite "
	    "WHERE (itemsite_id=:itemsite_id);");
  q.bindValue(":itemsite_id", list()->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params = parameterWidget()->parameters();
    params.append("item_id", q.value("itemsite_item_id"));
    params.append("run");

    dspUsageStatistics *newdlg = new dspUsageStatistics();
    SetResponse setresp = newdlg->set(params);
    if (setresp == NoError || setresp == NoError_Run)
      omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

