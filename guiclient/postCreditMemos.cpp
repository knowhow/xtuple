/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCreditMemos.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "distributeInventory.h"

postCreditMemos::postCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  _post->setFocus();
}

postCreditMemos::~postCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCreditMemos::languageChange()
{
  retranslateUi(this);
}

void postCreditMemos::sPost()
{
  q.exec( "SELECT cmhead_printed, COUNT(*) AS number "
          "FROM ( SELECT cmhead_id, cmhead_printed "
          "       FROM cmhead "
          "       WHERE (NOT cmhead_posted) ) AS data "
          "WHERE (checkCreditMemoSitePrivs(cmhead_id)) "
          "GROUP BY cmhead_printed;" );
  if (q.first())
  {
    int printed   = 0;
    int unprinted = 0;

    do
    {
      if (q.value("cmhead_printed").toBool())
        printed = q.value("number").toInt();
      else
        unprinted = q.value("number").toInt();
    }
    while (q.next());

    if ( ( (unprinted) && (!printed) ) && (!_postUnprinted->isChecked()) )
    {
      QMessageBox::warning( this, tr("No Credit Memos to Post"),
                            tr( "Although there are unposted Credit Memos, there are no unposted Credit Memos that have been printed.\n"
                                "You must manually print these Credit Memos or select 'Post Unprinted Credit Memos' before these Credit Memos\n"
                                "may be posted." ) );
      _postUnprinted->setFocus();
      return;
    }
  }
  else
  {
    QMessageBox::warning( this, tr("No Credit Memos to Post"),
                          tr("There are no Credit Memos, printed or not, to post.\n" ) );
    _close->setFocus();
    return;
  }

  q.exec("SELECT fetchJournalNumber('AR-CM') AS result");
  if (!q.first())
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  int journalNumber = q.value("result").toInt();
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT postCreditMemos(:postUnprinted, :journalNumber) AS result;");
  q.bindValue(":postUnprinted", QVariant(_postUnprinted->isChecked()));
  q.bindValue(":journalNumber", journalNumber);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result == -5)
    {
      rollback.exec();
      QMessageBox::critical( this, tr("Cannot Post one or more Credit Memos"),
                             tr( "The G/L Account Assignments for one or more of the Credit Memos that you are trying to post are not\n"
                                 "configured correctly.  Because of this, G/L Transactions cannot be posted for these Credit Memos.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post these Credit Memos." ) );
      return;
    }
    else if (result < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at postCreditMemos::%1, Error #%2.")
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Post Credit Memos"), tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");

    if (_printJournal->isChecked())
    {
      ParameterList params;
      params.append("journalNumber", journalNumber);

      orReport report("CreditMemoJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at postCreditMemos::%1.")
                       .arg(__LINE__) );
    return;
  }

  omfgThis->sCreditMemosUpdated();

  accept();
}
