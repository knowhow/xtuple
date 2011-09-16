/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailabilityByItem.h"

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
#include "inputManager.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspInventoryAvailabilityByItem::dspInventoryAvailabilityByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspInventoryAvailabilityByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability by Item"));
  setListLabel(tr("Availability"));
  setReportName("InventoryAvailabilityByItem");
  setMetaSQLOptions("inventoryAvailability", "general");
  setUseAltId(true);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);
  _showByGroupInt->addButton(_byDates);

  connect(_showReorder, SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  list()->addColumn(tr("Site"),         -1,         Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("LT"),           _whsColumn, Qt::AlignCenter,true, "itemsite_leadtime");
  list()->addColumn(tr("QOH"),          _qtyColumn, Qt::AlignRight, true, "qoh");
  list()->addColumn(tr("Allocated"),    _qtyColumn, Qt::AlignRight, true, "allocated");
  list()->addColumn(tr("Unallocated"),  _qtyColumn, Qt::AlignRight, true, "unallocated");
  list()->addColumn(tr("On Order"),     _qtyColumn, Qt::AlignRight, true, "ordered");
  list()->addColumn(tr("Reorder Lvl."), _qtyColumn, Qt::AlignRight, true, "reorderlevel");
  list()->addColumn(tr("OUT Level"),    _qtyColumn, Qt::AlignRight, true, "outlevel");
  list()->addColumn(tr("Available"),    _qtyColumn, Qt::AlignRight, true, "available");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();

  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  sHandleShowReorder(_showReorder->isChecked());
}

void dspInventoryAvailabilityByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(true);
  }

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
   _byDays->setChecked(true);
   _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
   _byDate->setChecked(true);
   _date->setDate(param.toDate());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspInventoryAvailabilityByItem::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Data"),
                         tr("<p>You must enter a valid Item Number."));
    _item->setFocus();
    return false;
  }

  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::warning(this, tr("Invalid Data"),
                         tr("<p>You have chosen to view Inventory "
                            "Availability as of a given date but have not "
                            "indicated the date. Please enter a valid date."));
    _date->setFocus();
    return false;
  }

  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

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

void dspInventoryAvailabilityByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Inventory History..."), this, SLOT(sViewHistory()));
  if (!_privileges->check("ViewInventoryHistory"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
  if (item->rawValue("allocated").toDouble() == 0.0)
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  if (item->rawValue("ordered").toDouble() == 0.0)
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));

  pMenu->addSeparator();

  if (((XTreeWidgetItem *)selected)->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
    if (!_privileges->check("MaintainPurchaseRequests"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Create P/O..."), this, SLOT(sCreatePO()));
    if (!_privileges->check("MaintainPurchaseOrders"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();
  }
  else if (((XTreeWidgetItem *)selected)->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Create W/O..."), this, SLOT(sCreateWO()));
    if (!_privileges->check("MaintainWorkOrders"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();
  }

  menuItem = pMenu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
  if (!_privileges->check("IssueCountTags"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()));
  if (!_privileges->check("EnterMiscCounts"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  pMenu->addAction(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()));

}

void dspInventoryAvailabilityByItem::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
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

void dspInventoryAvailabilityByItem::sViewOrders()
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

void dspInventoryAvailabilityByItem::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByItem::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  if (_leadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByItem::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByItem::sHandleShowReorder(bool pValue)
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(true);
}

