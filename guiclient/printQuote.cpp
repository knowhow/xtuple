/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printQuote.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"

printQuote::printQuote(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_number, SIGNAL(lostFocus()), this, SLOT(sHandleButtons()));
  connect(_print,    SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_report,  SIGNAL(newID(int)), this, SLOT(sHandleButtons()));

  _cust->setFocus();

  _captive  = false;
  _quheadid = -1; // TODO: replace with a QuoteCluster
}

printQuote::~printQuote()
{
}

void printQuote::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printQuote::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;
  _cust->setEnabled(false);

  QVariant param;
  bool     valid;

  param = pParams.value("quhead_id", &valid);
  if (valid)
  {
    _quheadid = param.toInt();
    populate();
  }

  return NoError;
}

void printQuote::sPrint()
{
  ParameterList params;
  params.append("quhead_id", _quheadid);

  orReport report(_report->code(), params);
  if (report.isValid())
  {
    if (report.print())
      emit finishedPrinting(_quheadid);
    if (_captive)
      accept();
  }
  else
  {
    report.reportError(this);
    if (_captive)
      reject();
  }

  _quheadid = -1;
  _number->clear();
  _cust->setId(-1);
  _cust->setFocus();
  _cust->setEnabled(true);
}

void printQuote::populate()
{
  q.prepare("SELECT quhead_number, quhead_cust_id,"
            "       findCustomerForm(quhead_cust_id, 'Q') AS reportname "
            "FROM quhead "
            "WHERE (quhead_id=:quhead_id);");
  q.bindValue(":quhead_id", _quheadid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("quhead_number").toString());
    _cust->setId(q.value("quhead_cust_id").toInt());
    _report->setCode(q.value("reportname").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printQuote::sHandleButtons()
{
  _print->setEnabled(! _number->text().isEmpty() && _report->isValid());
}
