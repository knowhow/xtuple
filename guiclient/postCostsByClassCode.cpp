/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCostsByClassCode.h"

#include <QSqlError>
#include <QVariant>
#include "submitAction.h"

postCostsByClassCode::postCostsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
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
}

postCostsByClassCode::~postCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCostsByClassCode::languageChange()
{
  retranslateUi(this);
}

void postCostsByClassCode::sSelectAll()
{
  _material->setChecked(TRUE);
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

void postCostsByClassCode::sPost()
{
  QString sql = "SELECT doPostCosts(item_id, TRUE, :material, :lowMaterial, "
                "       :directLabor, :lowDirectLabor, :overhead, :lowOverhead, "
                "       :machOverhead, :lowMachOverhead, :user, :lowUser, :rollUp) "
                "  FROM item, classcode "
                " WHERE(item_classcode_id=classcode_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id);";
  else if (_classCode->isPattern())
    sql += " AND (classcode_code ~ :classcode_pattern);";
  else
    sql = "SELECT doPostCosts(:material, :lowMaterial, "
          "    :directLabor, :lowDirectLabor, :overhead, :lowOverhead, "
          "    :machOverhead, :lowMachOverhead, :user, :lowUser, :rollUp);";

  q.prepare(sql);
  q.bindValue(":material",        _material->isChecked()          ? "t" : "f");
  q.bindValue(":lowMaterial",     _lowerMaterial->isChecked()     ? "t" : "f");
  q.bindValue(":directLabor",     _directLabor->isChecked()       ? "t" : "f");
  q.bindValue(":lowDirectLabor",  _lowerDirectLabor->isChecked()  ? "t" : "f");
  q.bindValue(":overhead",        _overhead->isChecked()          ? "t" : "f");
  q.bindValue(":lowOverhead",     _lowerOverhead->isChecked()     ? "t" : "f");
  q.bindValue(":machOverhead",    _machOverhead->isChecked()      ? "t" : "f");
  q.bindValue(":lowMachOverhead", _lowerMachOverhead->isChecked() ? "t" : "f");
  q.bindValue(":user",            _user->isChecked()              ? "t" : "f");
  q.bindValue(":lowUser",         _lowerUser->isChecked()         ? "t" : "f");
  q.bindValue(":rollUp",          _rollUp->isChecked()            ? "t" : "f");

  _classCode->bindValue(q);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void postCostsByClassCode::sSubmit()
{
  ParameterList params;

  params.append("action_name", "PostActualCost");

  if (_material->isChecked())
    params.append("Material");

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

