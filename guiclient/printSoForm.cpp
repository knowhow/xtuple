/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printSoForm.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"

printSoForm::printSoForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _so->setAllowedTypes(OrderLineEdit::Sales);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sPrint()));
  QPushButton *ok = _buttonBox->button(QDialogButtonBox::Ok);
  if(ok)
  {
    connect(_so, SIGNAL(valid(bool)), ok, SLOT(setEnabled(bool)));
    ok->setEnabled(false);
  }

  _captive = FALSE; 
  _report->populate( "SELECT form_id, form_name "
                     "FROM form "
                     "WHERE (form_key='SO') "
                     "ORDER BY form_name;" );
}

printSoForm::~printSoForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printSoForm::languageChange()
{
  retranslateUi(this);
}


enum SetResponse printSoForm::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _so->setId(param.toInt());
    _buttonBox->setFocus();
  }

  return NoError;
}

void printSoForm::sPrint()
{
  q.prepare( "SELECT form_report_name AS report_name "
             "  FROM form "
             " WHERE((form_id=:form_id));");
  q.bindValue(":form_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", _so->id());
    params.append("includeFormatted", true);
    params.append("excludeCancelled", true);

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
    {
      if (report.print())
        emit finishedPrinting(_so->id());
    }
    else
    {
      report.reportError(this);
      reject();
    }

    if (_captive)
      accept();
    else
    {
    _so->setId(-1);
    _so->setFocus();
    }
  }
  else
    QMessageBox::warning( this, tr("Could not locate report"),
                          tr("Could not locate the report definition the form \"%1\"")
                          .arg(_report->currentText()) );
}
