/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSlowMovingInventoryByClassCode.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"
#include "mqlutil.h"

dspSlowMovingInventoryByClassCode::dspSlowMovingInventoryByClassCode(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspSlowMovingInventoryByClassCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Slow Moving Inventory"));
  setListLabel(tr("Quantities on Hand"));
  setReportName("SlowMovingInventoryByClassCode");
  setMetaSQLOptions("slowMovingInventoryByClassCode", "detail");

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  _classCode->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Site"),          _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),   -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("UOM"),           _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Last Movement"), _itemColumn, Qt::AlignCenter, true,  "itemsite_datelastused" );
  list()->addColumn(tr("QOH"),           _qtyColumn,  Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  list()->addColumn(tr("Unit Cost"),     _costColumn, Qt::AlignRight,  true,  "cost"  );
  list()->addColumn(tr("Value"),         _costColumn, Qt::AlignRight,  true,  "value"  );

  sHandleValue(_showValue->isChecked());

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));
}

void dspSlowMovingInventoryByClassCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSlowMovingInventoryByClassCode::setParams(ParameterList & params)
{
  if(!_cutoffDate->isValid())
  {
    QMessageBox::warning(this, tr("No Cutoff Date"),
        tr("You must specify a cutoff date."));
    _cutoffDate->setFocus();
    return false;
  }

  params.append("cutoffDate", _cutoffDate->date());
  _warehouse->appendValue(params);
  _classCode->appendValue(params);

  if(_showValue->isChecked())
    params.append("showValue");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  return true;
}


void dspSlowMovingInventoryByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->addAction(tr("Transfer to another Site..."), this, SLOT(sTransfer()));;
    if (!_privileges->check("CreateInterWarehouseTrans"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Adjust this QOH..."), this, SLOT(sAdjust()));;
    if (!_privileges->check("CreateAdjustmentTrans"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Reset this QOH to 0..."), this, SLOT(sReset()));;
    if (!_privileges->check("CreateAdjustmentTrans"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Enter Misc. Count..."), this, SLOT(sMiscCount()));;
    if (!_privileges->check("EnterMiscCounts"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));;
    if (!_privileges->check("IssueCountTags"))
      menuItem->setEnabled(false);
  } 
}

void dspSlowMovingInventoryByClassCode::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSlowMovingInventoryByClassCode::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSlowMovingInventoryByClassCode::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSlowMovingInventoryByClassCode::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspSlowMovingInventoryByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspSlowMovingInventoryByClassCode::sHandleValue(bool pShowValue)
{
  list()->setColumnHidden(6, !pShowValue);
  list()->setColumnHidden(7, !pShowValue);
}

