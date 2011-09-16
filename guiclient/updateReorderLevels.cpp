/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "updateReorderLevels.h"
#include "mqlutil.h"
#include "submitAction.h"

updateReorderLevels::updateReorderLevels(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _daysGroupInt = new QButtonGroup(this);
    _daysGroupInt->addButton(_leadTime);
    _daysGroupInt->addButton(_fixedDays);

    connect(_post,    SIGNAL(clicked()),     this, SLOT(sPost()));
    connect(_preview, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
    connect(_results, SIGNAL(currentItemChanged(XTreeWidgetItem*, XTreeWidgetItem*)), this, SLOT(sCloseEdit(XTreeWidgetItem*,XTreeWidgetItem*)));
    connect(_results, SIGNAL(itemClicked(XTreeWidgetItem*, int)), this, SLOT(sOpenEdit(XTreeWidgetItem*, int)));
    connect(_submit,   SIGNAL(clicked()), this, SLOT(sSubmit()));
    connect(_update,   SIGNAL(clicked()), this, SLOT(sUpdate()));

    _results->addColumn(tr("Site"),        _whsColumn, Qt::AlignLeft, true, "reordlvl_warehous_code");
    _results->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft, true, "reordlvl_item_number");
    _results->addColumn(tr("Description"),         -1, Qt::AlignLeft, true, "reordlvl_item_descrip");
    _results->addColumn(tr("Leadtime"),    _qtyColumn, Qt::AlignRight,true, "reordlvl_leadtime");
    _results->addColumn(tr("Curr. Level"), _qtyColumn, Qt::AlignRight,true, "reordlvl_curr_level");
    _results->addColumn(tr("Days Stock"),  _qtyColumn, Qt::AlignRight,true, "reordlvl_daysofstock");
    _results->addColumn(tr("Total Usage"), _qtyColumn, Qt::AlignRight,true, "reordlvl_total_usage");
    _results->addColumn(tr("New Level"),   _qtyColumn, Qt::AlignRight,true, "reordlvl_calc_level");

    if (!_metrics->boolean("EnableBatchManager"))
      _submit->hide();
}

updateReorderLevels::~updateReorderLevels()
{
    // no need to delete child widgets, Qt does it all for us
}

void updateReorderLevels::languageChange()
{
    retranslateUi(this);
}

enum SetResponse updateReorderLevels::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    setWindowTitle("Update Reorder Levels by Class Code");
  }

  param = pParams.value("plancode", &valid);
  if (valid)
  {
    setWindowTitle("Update Reorder Levels by Planner Code");
    _parameter->setType(ParameterGroup::PlannerCode);
  }

  param = pParams.value("item", &valid);
  if (valid)
  {
    setWindowTitle("Update Reorder Level by Item");
    _stack->setCurrentIndex(1);
  }

  return NoError;
}

bool updateReorderLevels::setParams(ParameterList &params)
{
  if (_item->id() != -1)
    params.append("item_id", _item->id());
  else
    _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if (_leadTime->isChecked())
  {
    params.append("addLeadtime"),
    params.append("daysOfStock", _leadTimePad->value());
  }
  else if (_fixedDays->isChecked())
    params.append("daysOfStock", _days->value());

  params.append("period_id_list",_periods->periodList());

  return true;
}

void updateReorderLevels::sUpdate()
{
  _results->clear();
  _totalDays->setText("");

  QString method;
  if (_periods->topLevelItemCount() > 0)
  {
    QString sql;

    if (_preview->isChecked())
      method = "query";
    else
      method = "update";

    ParameterList params;
    if (! setParams(params))
      return;

    MetaSQLQuery mql = mqlLoad("updateReorderLevels", method);
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (_preview->isChecked())
    {
      _results->populate(q, true);
      if (q.first())
      {
        _totalDays->setText(q.value("reordlvl_total_days").toString());
        disconnect(_results, SIGNAL(itemChanged(XTreeWidgetItem*, int)), this, SLOT(sItemChanged(XTreeWidgetItem*, int)));
        connect(_results, SIGNAL(itemChanged(XTreeWidgetItem*, int)), this, SLOT(sItemChanged(XTreeWidgetItem*, int)));
        _tab->setCurrentIndex(1);
      }
      else
        QMessageBox::information(this, windowTitle(), tr("No item sites records found."));
    }
    else
      accept();
  }
  else
    QMessageBox::information(this, windowTitle(), tr("No Calendar Periods selected."));
}

void updateReorderLevels::sSubmit()
{
  if (_periods->topLevelItemCount() > 0)
  {
    ParameterList params;
    params.append("action_name", "UpdateReorderLevel");
    params.append("period_id_list", _periods->periodString());
    _warehouse->appendValue(params);
    if (_item->id() != -1)
      params.append("item_id", _item->id());
    else
      _parameter->appendValue(params);

    if (_leadTime->isChecked())
      params.append("leadtimepad", _leadTimePad->value());
    else if (_fixedDays->isChecked())
      params.append("fixedlookahead", _days->value());

    submitAction newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      accept();
  }
  else
    QMessageBox::information(this, windowTitle(), tr("No Calendar Periods selected."));
}

void updateReorderLevels::sHandleButtons()
{
  if (_preview->isChecked())
    _update->setText("Q&uery");
  else
    _update->setText("&Update");
}

void updateReorderLevels::sPost()
{
  MetaSQLQuery mql = mqlLoad("updateReorderLevels", "post");
  ParameterList params;
  QList<XTreeWidgetItem*> selected = _results->selectedItems();

  for (int i = 0; i < selected.size(); i++)
  {
    // Make sure editor is closed
    sCloseEdit(selected[i], selected[i]);
    params.clear();
    params.append("itemsite_id",           selected[i]->id());
    params.append("itemsite_reorderlevel", selected[i]->data(7,Qt::EditRole).toDouble());
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    delete selected[i];
  }
}

void updateReorderLevels::sOpenEdit(XTreeWidgetItem *item, const int col)
{
  if (col==7)
  {
    _results->openPersistentEditor(item,col);
    _results->editItem(item,col);
  }
}

void updateReorderLevels::sCloseEdit(XTreeWidgetItem * /*current*/, XTreeWidgetItem *previous)
{
  _results->closePersistentEditor(previous,7);
}

void updateReorderLevels::sItemChanged(XTreeWidgetItem *item, const int col)
{
  // Only positive numbers allowed
  if (col==7)
  {
    if (item->data(col,Qt::EditRole).toDouble() < 0)
      item->setData(col,Qt::EditRole,0);
    else
      item->setData(col,Qt::EditRole,item->data(col,Qt::EditRole).toDouble());
  }
}
