/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQOHByParameterList.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"

dspQOHByParameterList::dspQOHByParameterList(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspQOHByParameterList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quantities on Hand by Parameter List"));
  setListLabel(tr("Quantities on Hand"));
  setReportName("QOHByParameterList");
  setMetaSQLOptions("qoh", "detail");
  setUseAltId(true);

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  _showGroupInt = new QButtonGroup(this);
  _showGroupInt->addButton(_showAll);
  _showGroupInt->addButton(_showPositive);
  _showGroupInt->addButton(_showNegative);

  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  _asOf->setDate(omfgThis->dbDate(), true);

  list()->addColumn(tr("Site"),             _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Class Code"),       _itemColumn, Qt::AlignLeft,   true,  "classcode_code"   );
  list()->addColumn(tr("Item Number"),      _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),      -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("UOM"),              _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Default Location"), _itemColumn, Qt::AlignLeft,   true,  "defaultlocation"   );
  list()->addColumn(tr("Reorder Lvl."),     _qtyColumn,  Qt::AlignRight,  true,  "reorderlevel"  );
  list()->addColumn(tr("QOH"),              _qtyColumn,  Qt::AlignRight,  true,  "qoh"  );
  list()->addColumn(tr("Non-Netable"),      _qtyColumn,  Qt::AlignRight,  true,  "f_nnqoh"  );
  list()->addColumn(tr("Unit Cost"),        _costColumn, Qt::AlignRight,  true,  "cost"  );
  list()->addColumn(tr("Value"),            _costColumn, Qt::AlignRight,  true,  "value"  );
  list()->addColumn(tr("NN Value"),         _costColumn, Qt::AlignRight,  true,  "f_nnvalue"  );
  list()->addColumn(tr("Cost Method"),      _costColumn, Qt::AlignCenter, true,  "f_costmethod" );

  sHandleValue(_showValue->isChecked());

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));

  _asofGroup->hide(); // Issue #11793 - Not ready for this yet.
}

void dspQOHByParameterList::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

SetResponse dspQOHByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if(valid)
    _parameter->setType(ParameterGroup::ClassCode);

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

  param = pParams.value("itemgrp", &valid);
  if(valid)
    _parameter->setType(ParameterGroup::ItemGroup);

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

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  if (pParams.inList("run"))
    sFillList();

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Quantities on Hand by Class Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Quantities on Hand by Item Group"));
      break;

    default:
      break;
  }

  return NoError;
}

void dspQOHByParameterList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    QAction *menuItem;
  
    if (((XTreeWidgetItem *)pSelected)->altId())
    {
      pMenu->addAction(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewDetail()));
      pMenu->addSeparator();
    }

    if (_metrics->boolean("MultiWhs"))
    {
      menuItem = pMenu->addAction(tr("Transfer to another Site..."), this, SLOT(sTransfer()));;
      if (!_privileges->check("CreateInterWarehouseTrans"))
        menuItem->setEnabled(false);
    }

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

void dspQOHByParameterList::sViewDetail()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByParameterList::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByParameterList::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByParameterList::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByParameterList::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspQOHByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspQOHByParameterList::sHandleValue(bool pShowValue)
{
  list()->setColumnHidden(list()->column("cost"),         !pShowValue);
  list()->setColumnHidden(list()->column("value"),        !pShowValue);
  list()->setColumnHidden(list()->column("f_nnvalue"),    !pShowValue);
  list()->setColumnHidden(list()->column("f_costmethod"), !pShowValue);
}

void dspQOHByParameterList::sFillList()
{
  list()->clear();
  list()->setColumnVisible(list()->column("f_costmethod"),
                         _showValue->isChecked() && _usePostedCosts->isChecked());

  display::sFillList();
}

bool dspQOHByParameterList::setParams(ParameterList &params)
{
  params.append("byParameterList");
  params.append("asOf", _asOf->date());

  params.append("none", tr("None"));
  params.append("na", tr("N/A"));

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");
  else if (_useActualCosts->isChecked())
    params.append("useActualCosts");
  else if (_usePostedCosts->isChecked())
    params.append("usePostedCosts");

  if (_parameter->isSelected())
    params.append("byParameter");
  else if (_parameter->isPattern())
    params.append("byParameterPattern");
  else if(_parameter->type() == ParameterGroup::ItemGroup)
    params.append("byParameterType");

  _parameter->appendValue(params);

  if (_parameter->type() == ParameterGroup::ItemGroup)
    params.append("itemgrp");
  else if(_parameter->type() == ParameterGroup::ClassCode)
    params.append("classcode");

  _warehouse->appendValue(params);

  if (_showPositive->isChecked())
  {
    params.append("showPositive");
    params.append("onlyShowPositive"); // report
  }
  else if (_showNegative->isChecked())
  {
    params.append("showNegative");
    params.append("onlyShowNegative"); // report
  }

  if (_showValue->isChecked())
    params.append("showValue"); // report

  return true;
}
