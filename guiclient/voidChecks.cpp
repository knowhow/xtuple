/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voidChecks.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "storedProcErrorLookup.h"

voidChecks::voidChecks(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
  connect(_void,      SIGNAL(clicked()),  this, SLOT(sVoid()));

  _bankaccnt->setType(XComboBox::APBankAccounts);
  
  _numberOfChecks->setPrecision(0);
}

voidChecks::~voidChecks()
{
  // no need to delete child widgets, Qt does it all for us
}

void voidChecks::languageChange()
{
  retranslateUi(this);
}

void voidChecks::sVoid()
{
  q.prepare("SELECT checkhead_id, checkhead_number,"
	    "       voidCheck(checkhead_id) AS result"
            "  FROM checkhead"
            " WHERE ((NOT checkhead_posted)"
            "   AND  (NOT checkhead_replaced)"
            "   AND  (NOT checkhead_deleted)"
            "   AND  (NOT checkhead_void)"
            "   AND  (checkhead_bankaccnt_id=:bankaccnt_id))");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  while (q.next())
  {
    int result = q.value("result").toInt();
    if (result < 0)
      systemError(this,
		  tr("Check %1").arg(q.value("checkhead_number").toString()) +
		  "\n" + storedProcErrorLookup("voidCheck", result),
		  __FILE__, __LINE__);
    else if(_issueReplacements->isChecked())
    {
      XSqlQuery rplc;
      rplc.prepare("SELECT replaceVoidedCheck(:checkhead_id) AS result;");
      while(q.next())
      {
        rplc.bindValue(":checkhead_id", q.value("checkhead_id").toInt());
        rplc.exec();
      }
    }
    omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void voidChecks::sHandleBankAccount(int pBankaccntid)
{
  if (pBankaccntid == -1)
  {
    _checkNumber->clear();
    _numberOfChecks->clear();
  }
  else
  {
    q.prepare( "SELECT TEXT(MIN(checkhead_number)) as checknumber,"
               "       TEXT(COUNT(*)) AS numofchecks "
               "  FROM checkhead "
               " WHERE ( (NOT checkhead_void)"
               "   AND   (NOT checkhead_posted)"
               "   AND   (NOT checkhead_replaced)"
               "   AND   (NOT checkhead_deleted)"
               "   AND   (checkhead_bankaccnt_id=:bankaccnt_id) );" );
    q.bindValue(":bankaccnt_id", pBankaccntid);
    q.exec();
    if (q.first())
    {
      _checkNumber->setText(q.value("checknumber").toString());
      _numberOfChecks->setText(q.value("numofchecks").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
