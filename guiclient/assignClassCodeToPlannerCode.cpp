/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "assignClassCodeToPlannerCode.h"

#include <QMessageBox>
#include <QVariant>

assignClassCodeToPlannerCode::assignClassCodeToPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sAssign()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _classCode->setType(ParameterGroup::ClassCode);

  _planCode->setAllowNull(TRUE);
  _planCode->setType(XComboBox::PlannerCodes);
}

assignClassCodeToPlannerCode::~assignClassCodeToPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void assignClassCodeToPlannerCode::languageChange()
{
  retranslateUi(this);
}

void assignClassCodeToPlannerCode::sAssign()
{
  if(!_planCode->isValid())
  {
    QMessageBox::warning(this, tr("No Planner Code Selected"),
      tr("You must select a Planner Code to assign before continuing.") );
    return;
  }

  QString sql( "UPDATE itemsite "
               "SET itemsite_plancode_id=:plancode_id "
               "FROM item "
               "WHERE ( (itemsite_item_id=item_id)" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ");";

  q.prepare(sql);
  q.bindValue(":plancode_id", _planCode->id());
  q.bindValue(":classcode_id", _classCode->id());
  q.bindValue(":classcode_pattern", _classCode->pattern());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();

  accept();
}
