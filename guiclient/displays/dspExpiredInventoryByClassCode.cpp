/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspExpiredInventoryByClassCode.h"

#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include <metasql.h>
#include "mqlutil.h"

#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"
#include "guiclient.h"

#define COST_COL	6
#define VALUE_COL	7
#define METHOD_COL 8

dspExpiredInventoryByClassCode::dspExpiredInventoryByClassCode(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspExpiredInventoryByClassCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Expired Inventory"));
  setListLabel(tr("Expired Inventory"));
  setReportName("ExpiredInventoryByClassCode");
  setMetaSQLOptions("expiredInventory", "detail");

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  _classCode->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft,   true,  "ls_number"   );
  list()->addColumn(tr("Expiration"),   _dateColumn, Qt::AlignCenter, true,  "itemloc_expiration" );
  list()->addColumn(tr("Qty."),         _qtyColumn,  Qt::AlignRight,  true,  "itemloc_qty"  );
  list()->addColumn(tr("Unit Cost"),    _costColumn, Qt::AlignRight,  true,  "cost" );
  list()->addColumn(tr("Value"),        _costColumn, Qt::AlignRight,  true,  "value" );
  list()->addColumn(tr("Cost Method"),  _costColumn, Qt::AlignCenter, true,  "costmethod" );

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));
  if (! _privileges->check("ViewInventoryValue") || ! _showValue->isChecked())
  {
    list()->hideColumn(COST_COL);
    list()->hideColumn(VALUE_COL);
    list()->hideColumn(METHOD_COL);
  }
}

void dspExpiredInventoryByClassCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspExpiredInventoryByClassCode::sPopulateMenu(QMenu *, QTreeWidgetItem *, int)
{
#if 0
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Transfer to another Site..."), this, SLOT(sTransfer()), 0);
    if (!_privileges->check("CreateInterWarehouseTrans"))
      pMenu->setItemEnabled(menuItem, false);

    menuItem = pMenu->insertItem(tr("Adjust this QOH..."), this, SLOT(sAdjust()), 0);
    if (!_privileges->check("CreateAdjustmentTrans"))
      pMenu->setItemEnabled(menuItem, false);

    menuItem = pMenu->insertItem(tr("Reset this QOH to 0..."), this, SLOT(sReset()), 0);
    if (!_privileges->check("CreateAdjustmentTrans"))
      pMenu->setItemEnabled(menuItem, false);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Enter Misc. Count..."), this, SLOT(sMiscCount()), 0);
    if (!_privileges->check("EnterMiscCounts"))
      pMenu->setItemEnabled(menuItem, false);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
    if (!_privileges->check("IssueCountTags"))
      pMenu->setItemEnabled(menuItem, false);
  } 
#endif
}

void dspExpiredInventoryByClassCode::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspExpiredInventoryByClassCode::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspExpiredInventoryByClassCode::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspExpiredInventoryByClassCode::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspExpiredInventoryByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspExpiredInventoryByClassCode::sHandleValue(bool pShowValue)
{
  if (pShowValue)
  {
    list()->showColumn(COST_COL);
    list()->showColumn(VALUE_COL);
    list()->showColumn(METHOD_COL);
  }
  else
  {
    list()->hideColumn(COST_COL);
    list()->hideColumn(VALUE_COL);
    list()->hideColumn(METHOD_COL);
  }
}

void dspExpiredInventoryByClassCode::sFillList()
{
  list()->clear();
  list()->setColumnVisible(METHOD_COL, _showValue->isChecked() && _usePostedCosts->isChecked());

  display::sFillList();
}

bool dspExpiredInventoryByClassCode::setParams(ParameterList &params)
{
  if (_perishability->isChecked())
  {
    params.append("perishability");
    params.append("expiretype", tr("Perishability"));
  }
  else
  {
    params.append("warranty");
    params.append("expiretype", tr("Warranty"));
  }

  params.append("thresholdDays", _thresholdDays->value());

  _warehouse->appendValue(params);
  _classCode->appendValue(params);

  if(_showValue->isChecked())
    params.append("showValue");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");
  else if (_useActualCosts->isChecked())
    params.append("useActualCosts"); 
  else if (_usePostedCosts->isChecked())
    params.append("usePostedCosts");

  return true;
}

