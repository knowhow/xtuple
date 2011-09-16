/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reassignClassCodeByClassCode.h"

#include <QMessageBox>

reassignClassCodeByClassCode::reassignClassCodeByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_classCodePattern, SIGNAL(toggled(bool)), _classCode, SLOT(setEnabled(bool)));
  connect(_selectedClassCode, SIGNAL(toggled(bool)), _classCodes, SLOT(setEnabled(bool)));
  connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _classCodes->setType(XComboBox::ClassCodes);
  _newClassCode->setType(XComboBox::ClassCodes);
}

reassignClassCodeByClassCode::~reassignClassCodeByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void reassignClassCodeByClassCode::languageChange()
{
  retranslateUi(this);
}

void reassignClassCodeByClassCode::sReassign()
{
  if ( (_classCodePattern->isChecked()) && (_classCode->text().length() == 0) )
  {
    QMessageBox::warning(this, tr("Missing Class Code Pattern"),
                      tr("<p>You must enter a Class Code Pattern."));
    _classCode->setFocus();
    return;
  }
  
  QString sql( "UPDATE item "
               "SET item_classcode_id=:new_classcode_id ");

  if (_selectedClassCode->isChecked())
    sql += "WHERE (item_classcode_id=:old_classcode_id);";
  else if (_classCodePattern->isChecked())
    sql += "WHERE (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :old_classcode_code)));";

  q.prepare(sql);
  q.bindValue(":new_classcode_id", _newClassCode->id());
  q.bindValue(":old_classcode_id", _classCodes->id());
  q.bindValue(":old_classcode_code", _classCode->text());
  q.exec();

  accept();
}
