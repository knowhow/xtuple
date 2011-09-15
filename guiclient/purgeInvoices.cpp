/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purgeInvoices.h"

#include <QVariant>
#include <QMessageBox>
#include <QFile>
#include <QProgressDialog>

purgeInvoices::purgeInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_purge, SIGNAL(clicked()), this, SLOT(sPurge()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
}

purgeInvoices::~purgeInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void purgeInvoices::languageChange()
{
  retranslateUi(this);
}

void purgeInvoices::sPurge()
{
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cutoff Date"),
                          tr("You must enter a valid cutoff date before purging Invoice Records.") );
    return;
  }

  if ( QMessageBox::warning( this, tr("Delete Invoice Records"),
                             tr( "This function will purge Invoices and all of the associated documents including:\n"
                                 "Return Authorizations, Lot/Serial Registrations, Sales Orders, Shipments, and Billing.\n"
                                 "The results of the process are saved in the log file purgeInvoices.log.\n"
                                 "You will not be able to re-print an Invoice if you delete that Invoice's Records.\n"
                                 "\n"
                                 "Are you sure that you want to delete the selected Invoice Records?" ),
                             tr("Yes"), tr("No"), QString::null, 0, 1) == 0)
  {
    
    QString logpath;
    QString logname;
    QString logtogether;
    logpath = "";
    logname = "purgeInvoices.log";
    logtogether = logpath + logname;
    QFile logfile(logtogether);
    logfile.remove();
    logfile.open(QIODevice::WriteOnly);

    int _invoices = 1;
    int _kount = 1;
    
    XSqlQuery invoices;
    invoices.prepare("SELECT COUNT(*) AS invoice_count "
                     "FROM invchead "
                     "WHERE ( (invchead_invcdate <= :cutOffDate) "
                     "  AND   (invchead_posted) "
                     "  AND   (invchead_recurring_invchead_id!=invchead_id) "
                     "  AND   (checkInvoiceSitePrivs(invchead_id)) );");
    invoices.bindValue(":cutOffDate", _cutOffDate->date());
    invoices.exec();
    if (invoices.first())
      _invoices = (invoices.value("invoice_count").toInt());
    
    QProgressDialog _progress(tr("Purge Invoices in progress..."), tr("Cancel"), 0, _invoices, this);
    _progress.setWindowModality(Qt::WindowModal); 
            
    invoices.prepare("SELECT invchead_id, invchead_invcnumber "
                     "FROM invchead "
                     "WHERE ( (invchead_invcdate <= :cutOffDate) "
                     "  AND   (invchead_posted) "
                     "  AND   (NOT invchead_recurring) "
                     "  AND   (checkInvoiceSitePrivs(invchead_id)) );");
    invoices.bindValue(":cutOffDate", _cutOffDate->date());
    invoices.exec();
    while (invoices.next())
    {
      q.prepare("SELECT purgeInvoiceRecord(:cutOffDate, :invchead_id) AS result;");
      q.bindValue(":cutOffDate", _cutOffDate->date());
      q.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
      q.exec();
      if (q.first())
      {
        QString logmessage = "Invoice ";
        logmessage += invoices.value("invchead_invcnumber").toString();
        logmessage += ", result=";
        logmessage += q.value("result").toString();
        logfile.write(logmessage.toAscii());
        logfile.write("\n");
      }
      if (_progress.wasCanceled())
        break;
      _progress.setValue(_kount);
      _kount++;
    }
    _progress.setValue(_invoices);
    
    accept();
  }
}
