/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQOH.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "parameterwidget.h"

dspQOH::dspQOH(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspQOH", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quantities on Hand"));
  setReportName("QOH");
  setMetaSQLOptions("qoh", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  _showGroupInt = new QButtonGroup(this);
  _showGroupInt->addButton(_showAll);
  _showGroupInt->addButton(_showPositive);
  _showGroupInt->addButton(_showNegative);

  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Cost Category"), "costcat_id", XComboBox::CostCategories);
  parameterWidget()->append(tr("Cost Category Pattern"), "costcat_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->append(tr("Item Group Pattern"), "itemgrp_pattern", ParameterWidget::Text);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  list()->addColumn(tr("Site"),             _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),      _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),      -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Class Code"),       _itemColumn, Qt::AlignLeft,   true,  "classcode_code"   );
  list()->addColumn(tr("Cost Category"),    _itemColumn, Qt::AlignLeft,   false, "costcat_code"   );
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
}

void dspQOH::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

SetResponse dspQOH::set(const ParameterList &pParams)
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

  param = pParams.value("costcat_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Cost Category"), param);

  param = pParams.value("costcat_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Cost Category Pattern"), param);

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item Group"), param);

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item Group Pattern"), param);

  param = pParams.value("warehous_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Site"), param);

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
    sFillList();

  return NoError;
}

void dspQOH::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
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

    menuItem = pMenu->addAction(tr("Adjust this Quantity..."), this, SLOT(sAdjust()));;
    if (!_privileges->check("CreateAdjustmentTrans"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Reset this Quanity to 0..."), this, SLOT(sReset()));;
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

void dspQOH::sViewDetail()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOH::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOH::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOH::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOH::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspQOH::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspQOH::sHandleValue(bool pShowValue)
{
  list()->setColumnHidden(list()->column("cost"),         !pShowValue);
  list()->setColumnHidden(list()->column("value"),        !pShowValue);
  list()->setColumnHidden(list()->column("f_nnvalue"),    !pShowValue);
  list()->setColumnHidden(list()->column("f_costmethod"), !pShowValue);
}

void dspQOH::sFillList()
{
  list()->clear();
  list()->setColumnVisible(list()->column("f_costmethod"),
                         _showValue->isChecked() && _usePostedCosts->isChecked());

  display::sFillList();
}

bool dspQOH::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  params.append("none", tr("None"));
  params.append("na", tr("N/A"));

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");
  else if (_useActualCosts->isChecked())
    params.append("useActualCosts");
  else if (_usePostedCosts->isChecked())
    params.append("usePostedCosts");

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
