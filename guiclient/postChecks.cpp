/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postChecks.h"

#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"

postChecks::postChecks(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));

  _numberOfChecks->setPrecision(0);

  _bankaccnt->setAllowNull(TRUE);
  _bankaccnt->setType(XComboBox::APBankAccounts);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

}

postChecks::~postChecks()
{
  // no need to delete child widgets, Qt does it all for us
}

void postChecks::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postChecks::set(const ParameterList & pParams )
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  return NoError;
}

void postChecks::sPost()
{
  q.prepare("SELECT postChecks(:bankaccnt_id) AS result;");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
      systemError(this, storedProcErrorLookup("postChecks", result), __FILE__, __LINE__);

    omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);

    if (_printJournal->isChecked())
    {
      ParameterList params;
      params.append("journalNumber", result);

      orReport report("CheckJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }

    accept();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postChecks::sHandleBankAccount(int pBankaccntid)
{
  q.prepare( "SELECT COUNT(*) AS numofchecks "
             "FROM checkhead "
             "WHERE ( (NOT checkhead_void)"
             " AND (NOT checkhead_posted)"
             " AND (checkhead_printed)"
             " AND (checkhead_bankaccnt_id=:bankaccnt_id) );" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  if (q.first())
    _numberOfChecks->setDouble(q.value("numofchecks").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
