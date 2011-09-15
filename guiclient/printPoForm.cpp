/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPoForm.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"

printPoForm::printPoForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _report->populate( "SELECT form_id, form_name, form_name "
                     "FROM form "
                     "WHERE (form_key='PO') "
                     "ORDER BY form_name;" );
}

printPoForm::~printPoForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printPoForm::languageChange()
{
  retranslateUi(this);
}

void printPoForm::sPrint()
{
  q.prepare( "SELECT form_report_name AS report_name "
             "FROM form "
             "WHERE ( (form_id=:form_id) );");
  q.bindValue(":form_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("pohead_id", _po->id());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
    {
      if (report.print())
        emit finishedPrinting(_po->id());
    }
    else
    {
      report.reportError(this);
      reject();
    }

    _po->setId(-1);
    _po->setFocus();
  }
  else
    QMessageBox::warning( this, tr("Could not locate report"),
                          tr("Could not locate the report definition the form \"%1\"")
                          .arg(_report->currentText()) );
}
