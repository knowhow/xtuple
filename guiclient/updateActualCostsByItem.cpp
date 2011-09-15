/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateActualCostsByItem.h"

#include <QVariant>
#include <QSqlError>
#include "submitAction.h"

updateActualCostsByItem::updateActualCostsByItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_item, SIGNAL(valid(bool)), _update, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _submit, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  
  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  if (!_metrics->boolean("Routings"))
  {
    _directLabor->hide();
    _lowerDirectLabor->hide();
    _overhead->hide();
    _lowerOverhead->hide();
    _machOverhead->hide();
    _lowerMachOverhead->hide();
  }
  else if (_metrics->value("TrackMachineOverhead") != "M")
  {
    _machOverhead->setEnabled(FALSE);
    _machOverhead->setChecked(TRUE);
    _lowerMachOverhead->setEnabled(FALSE);
    _lowerMachOverhead->setChecked(TRUE);
  }

  _captive = FALSE;
  _updateActual = TRUE;
}

updateActualCostsByItem::~updateActualCostsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateActualCostsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse updateActualCostsByItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
    _captive = TRUE;
  }

  // default cost type is "actual"
  param = pParams.value("costtype", &valid);
  if (valid && param.toString() == "standard")
  {
    _updateActual = false;
    setWindowTitle(tr("Update Standard Costs By Item"));
    _rollUp->setText(tr("&Roll Up Standard Costs"));
  }

  return NoError;
}

void updateActualCostsByItem::sSelectAll()
{
  _lowerMaterial->setChecked(TRUE);
  _user->setChecked(TRUE);
  _lowerUser->setChecked(TRUE);
  if (_metrics->boolean("Routings"))
  {
    _directLabor->setChecked(TRUE);
    _lowerDirectLabor->setChecked(TRUE);
    _overhead->setChecked(TRUE);
    _lowerOverhead->setChecked(TRUE);
    if (_metrics->value("TrackMachineOverhead") == "M")
    {
      _machOverhead->setChecked(TRUE);
      _lowerMachOverhead->setChecked(TRUE);
    }
  }
}

void updateActualCostsByItem::sUpdate()
{
  XSqlQuery sql;
  sql.prepare("SELECT doUpdateCosts(:item_id, TRUE, :lowMaterial, :dirLabor, "
              "         :lowDirLabor, :overhead, :lowOverhead, :machOverhead, "
              "         :lowMachOverhead, :lowUser, :rollUp, :updateActual)");
  sql.bindValue(":item_id",         _item->id());
  sql.bindValue(":lowMaterial",     _lowerMaterial->isChecked()     ? "t" : "f");
  sql.bindValue(":dirLabor",        _directLabor->isChecked()       ? "t" : "f");
  sql.bindValue(":lowDirLabor",     _lowerDirectLabor->isChecked()  ? "t" : "f");
  sql.bindValue(":overhead",        _overhead->isChecked()          ? "t" : "f");
  sql.bindValue(":lowOverhead",     _lowerOverhead->isChecked()     ? "t" : "f");
  sql.bindValue(":machOverhead",    (_machOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  sql.bindValue(":lowMachOverhead", (_lowerMachOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  sql.bindValue(":lowUser",         _lowerUser->isChecked()         ? "t" : "f");
  sql.bindValue(":rollUp",          _rollUp->isChecked()            ? "t" : "f");
  sql.bindValue(":updateActual",    _updateActual                   ? "t" : "f" );

  sql.exec();
  if (sql.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__));
    return;
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));
    _item->setId(-1);
    _item->setFocus();
  }
}

void updateActualCostsByItem::sSubmit()
{
  ParameterList params;

  if (_updateActual)
    params.append("action_name", "UpdateActualCost");
  else
    params.append("action_name", "UpdateStandardCost");

  params.append("item_id", _item->id());

  if (_lowerMaterial->isChecked())
    params.append("LowerMaterial");

  if (_directLabor->isChecked())
    params.append("DirectLabor");

  if (_lowerDirectLabor->isChecked())
    params.append("LowerDirectLabor");

  if (_overhead->isChecked())
    params.append("Overhead");

  if (_lowerOverhead->isChecked())
    params.append("LowerOverhead");

  if (_machOverhead->isChecked())
    params.append("MachineOverhead");

  if (_lowerMachOverhead->isChecked())
    params.append("LowerMachineOverhead");

  if (_user->isChecked())
    params.append("User");

  if (_lowerUser->isChecked())
    params.append("LowerUser");

  if (_rollUp->isChecked())
    params.append("RollUp");

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();
}
