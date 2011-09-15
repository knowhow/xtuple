/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateActualCostsByClassCode.h"

#include <QSqlError>
#include <QVariant>
#include "submitAction.h"

updateActualCostsByClassCode::updateActualCostsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  
  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  _classCode->setType(ParameterGroup::ClassCode);

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

  _updateActual = true;
}

updateActualCostsByClassCode::~updateActualCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateActualCostsByClassCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse updateActualCostsByClassCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  // default cost type is "actual"
  param = pParams.value("costtype", &valid);
  if (valid)
  { 
    if (param.toString() == "standard")
    {
      _updateActual = false;
      setWindowTitle(tr("Update Standard Costs By Class Code"));
      _rollUp->setText(tr("&Roll Up Standard Costs"));
    }
    else if (param.toString() != "actual")
    {
      systemError(this, tr("A System Error occurred at %1::%2.\n"
                           "Illegal parameter value '%3' for 'costtype'")
                          .arg(__FILE__)
                          .arg(__LINE__)
                          .arg(param.toString()));
      return UndefinedError;
    }
  }

  return NoError;
}

void updateActualCostsByClassCode::sSelectAll()
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

void updateActualCostsByClassCode::sUpdate()
{
  QString sql = "SELECT doUpdateCosts(item_id, TRUE, :lowMaterial, :directLabor, "
                "       :lowDirectLabor, :overhead, :lowOverhead, "
                "       :machOverhead, :lowMachOverhead, :lowUser, :rollUp, "
                "       :updateActual ) "
                "  FROM item, classcode "
                " WHERE(item_classcode_id = classcode_id) ";

  if (_classCode->isSelected())
    sql += "  AND (item_classcode_id=:classcode_id);";
  else if (_classCode->isPattern())
    sql += "  AND (classcode_code ~ :classcode_pattern);";
  else
    sql = "SELECT doUpdateCosts(:lowMaterial, :directLabor, "
          "        :lowDirectLabor, :overhead, :lowOverhead, "
          "        :machOverhead, :lowMachOverhead, :lowUser, :rollUp, "
          "        :updateActual);";

  q.prepare(sql);
  q.bindValue(":lowMaterial",     _lowerMaterial->isChecked()     ? "t" : "f" );
  q.bindValue(":directLabor",     _directLabor->isChecked()       ? "t" : "f" );
  q.bindValue(":lowDirectLabor",  _lowerDirectLabor->isChecked()  ? "t" : "f" );
  q.bindValue(":overhead",        _overhead->isChecked()          ? "t" : "f" );
  q.bindValue(":lowOverhead",     _lowerOverhead->isChecked()     ? "t" : "f" );
  q.bindValue(":machOverhead",    (_machOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  q.bindValue(":lowMachOverhead", (_lowerMachOverhead->isChecked() ||
      ((_metrics->value("TrackMachineOverhead") != "M") && _metrics->boolean("Routings"))) ? "t" : "f");
  q.bindValue(":lowUser",         _lowerUser->isChecked()         ? "t" : "f" );
  q.bindValue(":rollUp",          _rollUp->isChecked()            ? "t" : "f" );
  q.bindValue(":updateActual",    _updateActual                   ? "t" : "f" );
  _classCode->bindValue(q);

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void updateActualCostsByClassCode::sSubmit()
{
  ParameterList params;

  if (_updateActual)
    params.append("action_name", "UpdateActualCost");
  else
    params.append("action_name", "UpdateStandardCost");

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

  _classCode->appendValue(params);

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();
}

