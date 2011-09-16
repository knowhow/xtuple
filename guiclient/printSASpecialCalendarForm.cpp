/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printSASpecialCalendarForm.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

printSASpecialCalendarForm::printSASpecialCalendarForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _report->populate( "SELECT form_id, form_name "
                     "FROM form "
                     "WHERE (form_key='SASC') "
                     "ORDER BY form_name;" );
}

printSASpecialCalendarForm::~printSASpecialCalendarForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printSASpecialCalendarForm::languageChange()
{
  retranslateUi(this);
}

void printSASpecialCalendarForm::sPrint()
{
  q.prepare( "SELECT form_report_name AS report_name "
             "  FROM form "
             " WHERE ( (form_id=:form_id) );");
  q.bindValue(":form_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("calhead_id", _calendar->id());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }
  }
  else
      QMessageBox::warning( this, tr("Could not locate report"),
                            tr("Could not locate the report definition the form \"%1\"")
                            .arg(_report->currentText()) );
}
