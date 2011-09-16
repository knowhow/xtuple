/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailabilityBySourceVendor.h"

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
#include "purchaseOrder.h"
#include "purchaseRequest.h"

dspInventoryAvailabilityBySourceVendor::dspInventoryAvailabilityBySourceVendor(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspInventoryAvailabilityBySourceVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability by Source Vendor"));
  setListLabel(tr("Availability"));
  setReportName("InventoryAvailabilityBySourceVendor");
  setMetaSQLOptions("inventoryAvailability", "general");

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);
  _showByGroupInt->addButton(_byDates);

  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));

  list()->addColumn(tr("Vendor #"),     _itemColumn, Qt::AlignLeft,  true, "vend_number");
  list()->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item"),         _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),  -1,          Qt::AlignLeft,  true, "itemdescrip");
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter,true, "itemsite_leadtime");
  list()->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight, true, "qoh");
  list()->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight, true, "allocated");
  list()->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight, true, "unallocated");
  list()->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight, true, "ordered");
  list()->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight, true, "reorderlevel");
  list()->addColumn(tr("OUT Level"),    _qtyColumn,  Qt::AlignRight, false, "outlevel");
  list()->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight, true, "available");
  
  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  sHandleShowReorder(_showReorder->isChecked());
}

void dspInventoryAvailabilityBySourceVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityBySourceVendor::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspInventoryAvailabilityBySourceVendor::setParams(ParameterList &params)
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

  if ((_byDates->isChecked()) && ( (!_startDate->isValid()) || (!_endDate->isValid()) ) )
  {
    QMessageBox::critical(this, tr("Enter Dates"),
                          tr("<p>You have choosen to view Inventory "
			     "Availability as of a given Start and End Date "
			     "but have not indicated the dates. Please enter "
			     "valid dates." ) );
    _startDate->setFocus();
    return false;
  }

  if (_preferences->boolean("ListNumericItemNumbersFirst"))
    params.append("ListNumericItemNumbersFirst");

  params.append("byVend");
  _warehouse->appendValue(params);
  _vendorGroup->appendValue(params);

  if (_leadTime->isChecked())
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

void dspInventoryAvailabilityBySourceVendor::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected, int)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  QAction *menuItem;

  menuItem = menu->addAction(tr("View Inventory History..."), this, SLOT(sViewHistory()));

  menu->addSeparator();

  menuItem = menu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
  if (item->rawValue("allocated").toDouble() == 0.0)
    menuItem->setEnabled(false);

  menuItem = menu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  if (item->rawValue("ordered").toDouble() == 0.0)
    menuItem->setEnabled(false);

  menuItem = menu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));

  menu->addSeparator();

  menuItem = menu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
  if (!_privileges->check("MaintainPurchaseRequests"))
    menuItem->setEnabled(false);

  menuItem = menu->addAction(tr("Create P/O..."), this, SLOT(sCreatePO()));
  if (!_privileges->check("MaintainPurchaseOrders"))
    menuItem->setEnabled(false);

  menu->addSeparator();

  menu->addAction(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()));

  menu->addSeparator();

  menuItem = menu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
  if (!_privileges->check("IssueCountTags"))
    menuItem->setEnabled(false);

  menuItem = menu->addAction(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()));
  if (!_privileges->check("EnterMiscCounts"))
    menuItem->setEnabled(false);
}

void dspInventoryAvailabilityBySourceVendor::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_leadTime->isChecked())
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

void dspInventoryAvailabilityBySourceVendor::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_leadTime->isChecked())
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

void dspInventoryAvailabilityBySourceVendor::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySourceVendor::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityBySourceVendor::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySourceVendor::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityBySourceVendor::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(true);
}
