/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "distributeInventory.h"
#include <openreports.h>

postInvoices::postInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  _post->setFocus();
}

postInvoices::~postInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void postInvoices::languageChange()
{
  retranslateUi(this);
}

void postInvoices::sPost()
{
  q.exec( "SELECT invchead_printed, COUNT(*) AS number "
          "FROM ( "
          "  SELECT * FROM invchead WHERE NOT (invchead_posted)) AS data "
          "WHERE (checkInvoiceSitePrivs(invchead_id)) "
          "GROUP BY invchead_printed;" );
  if (q.first())
  {
    int printed   = 0;
    int unprinted = 0;

    do
    {
      if (q.value("invchead_printed").toBool())
        printed = q.value("number").toInt();
      else
        unprinted = q.value("number").toInt();
    }
    while (q.next());

    if ( ( (unprinted) && (!printed) ) && (!_postUnprinted->isChecked()) )
    {
      QMessageBox::warning( this, tr("No Invoices to Post"),
                            tr( "Although there are unposted Invoices, there are no unposted Invoices that have been printed.\n"
                                "You must manually print these Invoices or select 'Post Unprinted Invoices' before these Invoices\n"
                                "may be posted." ) );
      _postUnprinted->setFocus();
      return;
    }
  }
  else
  {
    QMessageBox::warning( this, tr("No Invoices to Post"),
                          tr("There are no Invoices, printed or not, to post.\n" ) );
    _close->setFocus();
    return;
  }

  bool inclZero = false;
  q.exec("SELECT COUNT(invchead_id) AS numZeroInvcs "
         "FROM invchead "
         "WHERE ( (NOT invchead_posted) "
         "  AND   (invoiceTotal(invchead_id) <= 0.0) "
         "  AND   (checkInvoiceSitePrivs(invchead_id)) );");
  if (q.first() && q.value("numZeroInvcs").toInt() > 0)
  {
    int toPost = QMessageBox::question(this, tr("Invoices for 0 Amount"),
				       tr("There are %1 invoices with a total value of 0.\n"
					  "Would you like to post them?")
					 .arg(q.value("numZeroInvcs").toString()),
				       tr("Post All"), tr("Post Only Non-0"),
				       tr("Cancel"), 1, 2);
    if (toPost == 2)
      return;
    else if (toPost == 1)
      inclZero = false;
    else
      inclZero = true;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("SELECT fetchJournalNumber('AR-IN') AS journal;");
  if (!q.first())
  {
    systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                       .arg(__FILE__)
                       .arg(__LINE__)
                       .arg(q.value("journal").toInt()) );
    return;
  }
  int journalNumber = q.value("journal").toInt();

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT postInvoices(:postUnprinted, :inclZero, :journalNumber) AS result;");
  q.bindValue(":postUnprinted", QVariant(_postUnprinted->isChecked()));
  q.bindValue(":inclZero",      inclZero);
  q.bindValue(":journalNumber", journalNumber);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result == -5)
    {
      rollback.exec();
      QMessageBox::critical( this, tr("Cannot Post one or more Invoices"),
                             tr( "The G/L Account Assignments for one or more of the Invoices that you are trying to post are not\n"
                                 "configured correctly.  Because of this, G/L Transactions cannot be posted for these Invoices.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post these Invoices." ) );
      return;
    }
    else if (result < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Post Invoices"), tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");

    if (_printJournal->isChecked())
    {
      ParameterList params;
      params.append("journalNumber", journalNumber);

      orReport report("SalesJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }

    omfgThis->sInvoicesUpdated(-1, TRUE);
    omfgThis->sSalesOrdersUpdated(-1);

  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError( this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}
