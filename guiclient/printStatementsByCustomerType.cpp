/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printStatementsByCustomerType.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#define DEBUG false

printStatementsByCustomerType::printStatementsByCustomerType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _asOf->setDate(omfgThis->dbDate(), true);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = FALSE;

  _customerTypes->setType(ParameterGroup::CustomerType);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _dueonly->setChecked(true);
}

printStatementsByCustomerType::~printStatementsByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void printStatementsByCustomerType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printStatementsByCustomerType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printStatementsByCustomerType::sPrint()
{
  MetaSQLQuery custm("SELECT cust_id, (cust_number || '-' || cust_name) AS customer,"
                     "       findCustomerForm(cust_id, 'S') AS reportname "
                     "FROM "
					 "<? if exists(\"asofDate\") ?>"
                     "(SELECT araging_cust_id FROM araging(<? value (\"asofDate\") ?>, true))AS data, "
                     "<? endif ?> "
					 "custinfo, custtype, aropen "
                     "WHERE ( (cust_custtype_id=custtype_id)"
                     " AND (aropen_cust_id=cust_id)"
                     " AND (aropen_open)"
                     "<? if exists(\"graceDays\") ?>"
                     " AND (aropen_duedate < (CURRENT_DATE - <? value (\"graceDays\") ?>))"
                     "<? endif ?>"
                     "<? if exists(\"custtype_id\") ?>"
                     " AND (custtype_id=<? value (\"custtype_id\") ?>)"
                     "<? elseif exists(\"custtype_pattern\") ?>"
                     " AND (custtype_code ~ <? value (\"custtype_pattern\") ?>)"
                     "<? endif ?>"
                     "<? if exists(\"asofDate\") ?>"
                     " AND (cust_id = araging_cust_id)"
                     "<? endif ?>"
                     ") "
                     "GROUP BY cust_id, cust_number, cust_name "
                     "HAVING (SUM((aropen_amount - aropen_paid) *"
                     "             CASE WHEN (aropen_doctype IN ('C', 'R')) THEN -1"
                     "                  ELSE 1 END) > 0) "
                     "ORDER BY cust_number;" );

  ParameterList custp;
  _customerTypes->appendValue(custp);
  if (_graceDays->value() > 0)
    custp.append("graceDays", _graceDays->value());

  if (_dueonly->isChecked())
	  custp.append("graceDays", _graceDays->value());

  custp.append("asofDate", _asOf->date());

  XSqlQuery custq = custm.toQuery(custp);
  if (custq.first())
  {
    QPrinter printer(QPrinter::HighResolution);
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }
    bool doSetup = true;
    do
    {
      if (DEBUG)
        qDebug("printing statement for %s",
               qPrintable(custq.value("customer").toString()));

      message( tr("Printing Statement for Customer %1.")
               .arg(custq.value("customer").toString()) );

      ParameterList params;
      params.append("invoice", tr("Invoice"));
      params.append("debit", tr("Debit Memo"));
      params.append("credit", tr("Credit Memo"));
      params.append("deposit", tr("Deposit"));
      params.append("cust_id", custq.value("cust_id").toInt());
      params.append("asofdate", _asOf->date());

      if (DEBUG) qDebug("instantiating report");
      orReport report(custq.value("reportname").toString(), params);
      if (! (report.isValid() && report.print(&printer, doSetup)) )
      {
        report.reportError(this);
	orReport::endMultiPrint(&printer);
        reject();
      }
      doSetup = false;

      if (DEBUG)
        qDebug("emitting finishedPrinting(%d)", custq.value("cust_id").toInt());
      emit finishedPrinting(custq.value("cust_id").toInt());
    }
    while (custq.next());
    orReport::endMultiPrint(&printer);

    message("");
  }
  else if (custq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, custq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::information( this, tr("No Statement to Print"),
                              tr("<p>There are no Customers whose accounts are "
                                 "past due within the specified AsOf date for "
                                 "which Statements should be printed.") );

  if (_captive)
    accept();
  else
    _close->setText(tr("&Close"));
}
