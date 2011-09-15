/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailability.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistory.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "postMiscProduction.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"
#include "parameterwidget.h"

dspInventoryAvailability::dspInventoryAvailability(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspInventoryAvailability", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability"));
  setReportName("InventoryAvailability");
  setMetaSQLOptions("inventoryAvailability", "general");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  _forgetful = true;
  if(window())
    _settingsName = window()->objectName() + "/asof";
  _asof->setCurrentIndex(_preferences->value(_settingsName).toInt());
  _forgetful = false;

  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->append(tr("Item Group Pattern"), "itemgrp_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Planner Code"), "plancode_id", XComboBox::PlannerCodes);
  parameterWidget()->append(tr("Planner Code Pattern"), "plancode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Source Vendor"), "vend_id", ParameterWidget::Vendor);
  parameterWidget()->appendComboBox(tr("Source Vendor Type"), "vendtype_id", XComboBox::VendorTypes);
  parameterWidget()->append(tr("Source Vendor Type Pattern"), "vendtype_pattern", ParameterWidget::Text);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  sHandleShowReorder(_showReorder->isChecked());
  sByVendorChanged();

  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
  connect(_byVendor, SIGNAL(toggled(bool)), this, SLOT(sByVendorChanged()));
  connect(_asof, SIGNAL(currentIndexChanged(int)), this, SLOT(sAsofChanged(int)));
}

void dspInventoryAvailability::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailability::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  parameterWidget()->setSavedFilters();

  param = pParams.value("classcode_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Class Code"), param);

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Class Code Pattern"), param);

  param = pParams.value("plancode_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Planner Code"), param);

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Planner Code Pattern"), param);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item Group"), param);

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item Group Pattern"), param);

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param);

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    XSqlQuery qry;
    qry.prepare("SELECT itemsite_item_id, itemsite_warehous_id "
                "FROM itemsite "
                "WHERE itemsite_id=:itemsite_id;");
    qry.bindValue(":itemsite_id", param.toInt());
    qry.exec();
    if (qry.first())
    {
      parameterWidget()->setDefault(tr("Item"), qry.value("itemsite_item_id"));
      parameterWidget()->setDefault(tr("Site"), qry.value("itemsite_warehous_id"));
    }
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Site"), param);

  param = pParams.value("byDays", &valid);
  if (valid)
  {
    _forgetful = true;
    _asof->setCurrentIndex(LOOKAHEADDAYS);
    _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
    _forgetful = true;
    _asof->setCurrentIndex(CUTOFFDATE);
    _date->setDate(param.toDate());
  }

  // sources may not exist, so turn by Vendor off to avoid confusion
  _byVendor->setForgetful(true);
  _byVendor->setChecked(false);

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspInventoryAvailability::setParams(ParameterList &params)
{
  if ((_asof->currentIndex() == CUTOFFDATE) && (!_date->isValid()))
  {
    QMessageBox::critical(this, tr("Enter Valid Date"),
                          tr("Please enter a valid date."));
    _date->setFocus();
    return false;
  }
  else if ((_asof->currentIndex() == DATES) && (!_startDate->isValid() || !_endDate->isValid()))
  {
    QMessageBox::critical(this, tr("Enter Valid Dates"),
                          tr("Please select a valid date range."));
    _date->setFocus();
    return false;
  }

  if (!display::setParams(params))
    return false;

  if (_asof->currentIndex() == ITEMSITELEADTIME)
    params.append("byLeadTime");
  else if (_asof->currentIndex() == LOOKAHEADDAYS)
    params.append("byDays", _days->text().toInt());
  else if (_asof->currentIndex() == CUTOFFDATE)
    params.append("byDate", _date->date());
  else if (_asof->currentIndex() == DATES)
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

  if(_byVendor->isChecked())
    params.append("byVend");

  return true;
}

void dspInventoryAvailability::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected, int)
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
    menuItem = menu->addAction(tr("Create Purchase Request..."), this, SLOT(sCreatePR()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));

    menuItem = menu->addAction(tr("Create Purchase Order..."), this, SLOT(sCreatePO()));
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

void dspInventoryAvailability::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  dspInventoryHistory *newdlg = new dspInventoryHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_asof->currentIndex() == ITEMSITELEADTIME)
    params.append("byLeadTime", true);
  else if (_asof->currentIndex() == LOOKAHEADDAYS)
    params.append("byDays", _days->value());
  else if (_asof->currentIndex() == CUTOFFDATE)
    params.append("byDate", _date->date());
  else if (_asof->currentIndex() == DATES)
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_asof->currentIndex() == ITEMSITELEADTIME)
    params.append("byLeadTime", true);
  else if (_asof->currentIndex() == LOOKAHEADDAYS)
    params.append("byDays", _days->value());
  else if (_asof->currentIndex() == CUTOFFDATE)
    params.append("byDate", _date->date());
  else if (_asof->currentIndex() == DATES)
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sPostMiscProduction()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  postMiscProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailability::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailability::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_asof->currentIndex() == ITEMSITELEADTIME)
    params.append("byLeadTime", true);
  else if (_asof->currentIndex() == LOOKAHEADDAYS)
    params.append("byDays", _days->value());
  else if (_asof->currentIndex() == CUTOFFDATE)
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailability::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailability::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailability::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(true);
}

void dspInventoryAvailability::sByVendorChanged()
{
  list()->clear();
  list()->setColumnCount(0);

  if (_byVendor->isChecked())
  {
    list()->addColumn(tr("Vendor #"),     _itemColumn, Qt::AlignLeft,  true, "vend_number");
    setReportName("InventoryAvailabilityBySourceVendor");
  }
  else
    setReportName("InventoryAvailability");
  list()->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item"),         _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),  -1,          Qt::AlignLeft,  true, "itemdescrip");
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter,true, "itemsite_leadtime");
  list()->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight, true, "qoh");
  list()->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight, true, "allocated");
  list()->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight, true, "unallocated");
  list()->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight, true, "ordered");
  list()->addColumn(tr("PO Requests"),  _qtyColumn,  Qt::AlignRight, true, "requests");
  list()->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight, true, "reorderlevel");
  list()->addColumn(tr("OUT Level"),    _qtyColumn,  Qt::AlignRight, false, "outlevel");
  list()->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight, true, "available");
}

void dspInventoryAvailability::sAsofChanged(int index)
{
  if (!_forgetful)
    _preferences->set(_settingsName, index);
}
