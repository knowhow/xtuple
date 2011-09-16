/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSitesByParameterList.h"

#include <QVariant>
#include <QWorkspace>
#include <QAction>
#include <QMenu>

#include "createCountTagsByItem.h"
#include "dspInventoryAvailabilityByItem.h"
#include "itemSite.h"

dspItemSitesByParameterList::dspItemSitesByParameterList(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemSitesByParameterList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Sites by Class Code"));
  setListLabel(tr("Item Sites"));
  setReportName("ItemSitesByParameterList");
  setMetaSQLOptions("itemSites", "detail");

  _parameter->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Site"),          _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),   -1,           Qt::AlignLeft,   true,  "description"   );
  list()->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("QOH"),           _qtyColumn,   Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  list()->addColumn(tr("Loc. Cntrl."),   _dateColumn,  Qt::AlignCenter, true,  "loccntrl" );
  list()->addColumn(tr("Cntrl. Meth."),  _dateColumn,  Qt::AlignCenter, true,  "controlmethod" );
  list()->addColumn(tr("Sold Ranking"),  _dateColumn,  Qt::AlignCenter, true,  "soldranking" );
  list()->addColumn(tr("ABC Class"),     _dateColumn,  Qt::AlignCenter, true,  "itemsite_abcclass" );
  list()->addColumn(tr("Cycle Cnt."),    _dateColumn,  Qt::AlignCenter, true,  "itemsite_cyclecountfreq" );
  list()->addColumn(tr("Last Cnt'd"),    _dateColumn,  Qt::AlignCenter, true,  "datelastcount" );
  list()->addColumn(tr("Last Used"),     _dateColumn,  Qt::AlignCenter, true,  "datelastused" );
}

void dspItemSitesByParameterList::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspItemSitesByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  _showInactive->setChecked(pParams.inList("showInactive"));

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

  param = pParams.value("costcat_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CostCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("costcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CostCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("costcat", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CostCategory);

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Item Sites by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Item Sites by Planner Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Item Sites by Item Group"));
      break;

    case ParameterGroup::CostCategory:
      setWindowTitle(tr("Item Sites by Cost Category"));
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

void dspItemSitesByParameterList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");
  params.append("byLeadTime");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Item Site..."), this, SLOT(sView()));;
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Edit Item Site..."), this, SLOT(sEdit()));;
  if (!_privileges->check("MaintainItemSites"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Inventory Availability..."), this, SLOT(sInventoryAvailability()));;
  if (!_privileges->check("ViewInventoryAvailability"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));;
  if (!_privileges->check("IssueCountTags"))
    menuItem->setEnabled(false);
}

bool dspItemSitesByParameterList::setParams(ParameterList &params)
{
  params.append("byParameterList");  

  params.append("regular", tr("Regular"));
  params.append("none", tr("None"));
  params.append("lot", tr("Lot #"));
  params.append("serial", tr("Serial #"));
  params.append("na", tr("N/A"));
  
  if (_showInactive->isChecked())
    params.append("showInactive");  
  _warehouse->appendValue(params);

  if (_parameter->isSelected())
    params.append("byParameterId");
  else if (_parameter->isPattern())
    params.append("byParameterPattern");
  _parameter->appendValue(params);

  return true;
}
