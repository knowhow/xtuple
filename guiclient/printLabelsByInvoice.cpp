/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printLabelsByInvoice.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

printLabelsByInvoice::printLabelsByInvoice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invoiceNumber, SIGNAL(lostFocus()), this, SLOT(sParseInvoiceNumber()));
  connect(_from, SIGNAL(valueChanged(int)), this, SLOT(sSetToMin(int)));

  _invcheadid = -1;

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );
}

printLabelsByInvoice::~printLabelsByInvoice()
{
  // no need to delete child widgets, Qt does it all for us
}

void printLabelsByInvoice::languageChange()
{
  retranslateUi(this);
}

void printLabelsByInvoice::sPrint()
{
  XSqlQuery query;
  query.prepare( "SELECT labelform_report_name AS report_name "
                 "FROM labelform "
                 "WHERE ( (labelform_id=:labelform_id) );" );
  query.bindValue(":labelform_id", _report->id());
  query.exec();
  if (query.first())
  {
    ParameterList params;
    params.append("invchead_id", _invcheadid);
    params.append("labelFrom", _from->value());
    params.append("labelTo", _to->value());

    orReport report(query.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    _invoiceNumber->clear();
    _invoiceNumber->setFocus();
  }
  else
    QMessageBox::warning(this, tr("Could not locate report"),
                         tr("Could not locate the report definition the form \"%1\"")
                         .arg(_report->currentText()) );
}

void printLabelsByInvoice::sParseInvoiceNumber()
{
  XSqlQuery query;
  query.prepare( "SELECT invchead_id "
                 "FROM invchead "
                 "WHERE (invchead_invcnumber=:invoiceNumber);" );
  query.bindValue(":invoiceNumber", _invoiceNumber->text());
  query.exec();
  if (query.first())
  {
    _print->setEnabled(TRUE);

    _invcheadid = query.value("invchead_id").toInt();
  }
  else
  {
    _print->setEnabled(FALSE);
    _invoiceNumber->clear();
    _invoiceNumber->setFocus();

    _invcheadid = -1;
  }
}

void printLabelsByInvoice::sSetToMin(int pValue)
{
  _to->setMinimum(pValue);
}
