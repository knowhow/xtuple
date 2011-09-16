/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailabilityByParameterList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistoryByItem.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "postMiscProduction.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspInventoryAvailabilityByParameterList::dspInventoryAvailabilityByParameterList(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspInventoryAvailabilityByParameterList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability by Parameter List"));
  setListLabel(tr("Availability"));
  setReportName("InventoryAvailabilityByParameterList");
  setMetaSQLOptions("inventoryAvailability", "general");
  setUseAltId(true);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_byLeadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);
  _showByGroupInt->addButton(_byDates);

  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  list()->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),  -1,          Qt::AlignLeft,  true, "itemdescrip");
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter,true, "itemsite_leadtime");
  list()->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight, true, "qoh");
  list()->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight, true, "allocated");
  list()->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight, true, "unallocated");
  list()->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight, true, "ordered");
  list()->addColumn(tr("PO Requests"),  _qtyColumn,  Qt::AlignRight, true, "requests");
  list()->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight, true, "reorderlevel");
  list()->addColumn(tr("OUT Level."),   _qtyColumn,  Qt::AlignRight, false, "outlevel");
  list()->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight, true, "available");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  sHandleShowReorder(_showReorder->isChecked());
}

void dspInventoryAvailabilityByParameterList::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ItemGroup);

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Inventory Availability by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Inventory Availability by Planner Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Inventory Availability by Item Group"));
      break;

    default:
      break;
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspInventoryAvailabilityByParameterList::setParams(ParameterList &params)
{
  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::critical(this, tr("Enter Valid Date"),
                          tr("<p>You have choosen to view Inventory "
			     "Availability as of a given date but have not "
			     "indicated the date. Please enter a valid date."));
    _date->setFocus();
    return false;
  }

  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if (_parameter->isAll())
  {
    if (_parameter->type() == ParameterGroup::ItemGroup)
      params.append("itemgrp");
    else if(_parameter->type() == ParameterGroup::PlannerCode)
      params.append("plancode");
    else if (_parameter->type() == ParameterGroup::ClassCode)
      params.append("classcode");
  }

  if (_byLeadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->text().toInt());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byDates");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  if(_showReorder->isChecked())
    params.append("showReorder");

  if(_ignoreReorderAtZero->isChecked())
    params.append("ignoreReorderAtZero");

  if(_showShortages->isChecked())
    params.append("showShortages");

  return true;
}

void dspInventoryAvailabilityByParameterList::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected, int)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  QAction *menuItem;

  menuItem = menu->addAction(tr("View Inventory History..."), this, SLOT(sViewHistory()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));

  menu->addSeparator();

  menuItem = menu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
  menuItem->setEnabled(item->rawValue("allocated").toDouble() != 0.0);

  menuItem = menu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  menuItem->setEnabled(item->rawValue("ordered").toDouble() != 0.0);

  menuItem = menu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));

  menu->addSeparator();

  if (((XTreeWidgetItem *)selected)->altId() == 1)
  {
    menuItem = menu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));

    menuItem = menu->addAction(tr("Create P/O..."), this, SLOT(sCreatePO()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menu->addSeparator();
  }
  else if (((XTreeWidgetItem *)selected)->altId() == 2)
  {
    menuItem = menu->addAction(tr("Create W/O..."), this, SLOT(sCreateWO()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

    menuItem = menu->addAction(tr("Post Misc. Production..."), this, SLOT(sPostMiscProduction()));
    menuItem->setEnabled(_privileges->check("PostMiscProduction"));

    menu->addSeparator();
  }
    
  menu->addAction(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()));

  menu->addSeparator();

  menuItem = menu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
  menuItem->setEnabled(_privileges->check("IssueCountTags"));

  menuItem = menu->addAction(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()));
  menuItem->setEnabled(_privileges->check("EnterMiscCounts"));
}

void dspInventoryAvailabilityByParameterList::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_byLeadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_byLeadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sPostMiscProduction()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  postMiscProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_byLeadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByParameterList::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(true);
}

