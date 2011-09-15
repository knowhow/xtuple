/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSites.h"

#include <QVariant>
#include <QWorkspace>
#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QMessageBox>

#include "createCountTagsByItem.h"
#include "dspInventoryAvailability.h"
#include "itemSite.h"
#include "parameterwidget.h"
#include "storedProcErrorLookup.h"

itemSites::itemSites(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "itemSites", fl)
{
  setWindowTitle(tr("Item Sites"));
  setReportName("ItemSites");
  setMetaSQLOptions("itemSites", "detail");
  setParameterWidgetVisible(true);
  setSearchVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Cost Category"), "costcat_id", XComboBox::CostCategories);
  parameterWidget()->append(tr("Cost Category Pattern"), "costcat_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Planner Code"), "plancode_id", XComboBox::PlannerCodes);
  parameterWidget()->append(tr("Planner Code Pattern"), "plancode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  if (_privileges->check("MaintainItemSites"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  list()->addColumn(tr("Site"),          _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),   -1,           Qt::AlignLeft,   true,  "description"   );
  list()->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("QOH"),           _qtyColumn,   Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  list()->addColumn(tr("Active"),        _ynColumn,    Qt::AlignCenter, true,  "itemsite_active" );
  list()->addColumn(tr("Loc. Cntrl."),   _dateColumn,  Qt::AlignCenter, true,  "itemsite_loccntrl" );
  list()->addColumn(tr("Cntrl. Method"), _dateColumn,  Qt::AlignCenter, true,  "controlmethod" );
  list()->addColumn(tr("Sold Ranking"),  _dateColumn,  Qt::AlignCenter, false,  "soldranking" );
  list()->addColumn(tr("ABC Class"),     _dateColumn,  Qt::AlignCenter, false,  "itemsite_abcclass" );
  list()->addColumn(tr("Cycle Cnt."),    _dateColumn,  Qt::AlignCenter, false,  "itemsite_cyclecountfreq" );
  list()->addColumn(tr("Last Cnt'd"),    _dateColumn,  Qt::AlignCenter, false,  "datelastcount" );
  list()->addColumn(tr("Last Used"),     _dateColumn,  Qt::AlignCenter, false,  "datelastused" );
}

enum SetResponse itemSites::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    if (param.toInt() > 0)
      parameterWidget()->setDefault(tr("Site"), param);
  }

  if (pParams.inList("showInactive"))
    parameterWidget()->setDefault(tr("Show Inactive"), true);

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

  param = pParams.value("costcat_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Cost Category"), param);

  param = pParams.value("costcat_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Cost Category Pattern"), param);

  parameterWidget()->applyDefaultFilterSet();;

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void itemSites::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sCopy()
{
  q.prepare("SELECT copyItemSite(:olditemsiteid, NULL) AS result;");
  q.bindValue(":olditemsiteid", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("copyItemSite", result), __FILE__, __LINE__);
      return;
    }
    ParameterList params;
    params.append("mode", "edit");
    params.append("itemsite_id", result);

    itemSite newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Accepted)
    {
      q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
      q.bindValue(":itemsite_id", result);
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteItemSite", result), __FILE__, __LINE__);
          return;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Line Item?"),
                            tr("Are you sure that you want to delete the "
                               "selected Item Site?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteItemSite", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");
  params.append("byLeadTime");

  dspInventoryAvailability *newdlg = new dspInventoryAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemSites::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));;
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));;
  if (!_privileges->check("MaintainItemSites"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Copy..."), this, SLOT(sCopy()));;
  if (!_privileges->check("MaintainItemSites"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));;
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

bool itemSites::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  params.append("regular", tr("Regular"));
  params.append("none", tr("None"));
  params.append("lot", tr("Lot #"));
  params.append("serial", tr("Serial #"));
  params.append("na", tr("N/A"));
  params.append("never", tr("Never"));

  return true;
}
