/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "submitReport.h"

#include <QVariant>
#include <QMessageBox>
#include <xvariant.h>

submitReport::submitReport(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_scheduled, SIGNAL(toggled(bool)), _time, SLOT(setEnabled(bool)));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_scheduled, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _error = 0;

  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    _fromEmail->setText(q.value("usr_email"));
}

submitReport::~submitReport()
{
  // no need to delete child widgets, Qt does it all for us
}

void submitReport::languageChange()
{
  retranslateUi(this);
}

enum SetResponse submitReport::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _params = pParams;

  QVariant param;
  bool     valid;

  param = pParams.value("report_name", &valid);
  if (valid)
  {
    q.prepare( "SELECT report_name "
               "FROM report "
               "WHERE (report_name=:report_name);" );
    q.bindValue(":report_name", param.toString());
    q.exec();
    if (q.first())
    {
      _documentName->setText(q.value("report_name").toString());
      _cachedReportName = q.value("report_name").toString();
    }
    else
      _error = cNoReportDefinition;
  }

  param = pParams.value("report_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT report_name "
               "FROM report "
               "WHERE (report_id=:report_id);" );
    q.bindValue(":report_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _documentName->setText(q.value("report_name").toString());
      _cachedReportName = q.value("report_name").toString();
    }
    else
      _error = cNoReportDefinition;
  }

  param = pParams.value("responseEmail", &valid);
  if (valid)
    _email->setText(param.toString());
  else
  {
    q.prepare( "SELECT usr_email "
               "FROM report, usr "
               "WHERE (usr_username=CURRENT_USER);" );
    q.exec();
    if (q.first())
      _email->setText(q.value("usr_email").toString());
  }
  
  param = pParams.value("responseBody", &valid);
  if (valid)
    _responseBody->setPlainText(param.toString());

  return NoError;
}

int submitReport::check()
{
  return _error;
}

void submitReport::sSubmit()
{
  if (_email->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Submit Report"),
                           tr("<p>You must indicate an Email address to which "
                              "the completed Report will be sent.") );
    _email->setFocus();
    return;
  }

  if (_asap->isChecked())
    q.prepare("SELECT submitReportToBatch(:reportName, :fromEmail, :emailAddress, '', :responseBody, '', CURRENT_TIMESTAMP) AS batch_id;");
  else
  {
    q.prepare("SELECT submitReportToBatch(:reportName, :fromEmail, :emailAddress, '', :responseBody, '', :scheduled) AS batch_id;");

    QDateTime scheduled;
    scheduled.setDate(_date->date());
    scheduled.setTime(_time->time());
    q.bindValue(":scheduled", scheduled);
  }

  q.bindValue(":reportName", _cachedReportName);
  q.bindValue(":fromEmail", _fromEmail->text());
  q.bindValue(":emailAddress", _email->text());
  q.bindValue(":responseBody", _responseBody->toPlainText());
  q.exec();
  if (q.first())
  {
    int batchid = q.value("batch_id").toInt();

    q.prepare( "INSERT INTO batchparam "
               "( batchparam_batch_id, batchparam_order,"
               "  batchparam_name, batchparam_type, batchparam_value ) "
               "VALUES "
               "( :batchparam_batch_id, :batchparam_order,"
               "  :batchparam_name, :batchparam_type, :batchparam_value );" );
    for (int counter = 0; counter < _params.count(); counter++)
    {
      q.bindValue(":batchparam_batch_id", batchid);
      q.bindValue(":batchparam_order", (counter + 1));
      q.bindValue(":batchparam_name", _params.name(counter));
      QVariant v = _params.value(counter);
      q.bindValue(":batchparam_type", QVariant::typeToName(v.type()));
      q.bindValue(":batchparam_value", XVariant::encode(v));
      q.exec();
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );

  accept();
}

