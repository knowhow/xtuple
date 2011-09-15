/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printItemLabelsByClassCode.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>

printItemLabelsByClassCode::printItemLabelsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = FALSE;

  _classCode->setType(ParameterGroup::ClassCode);

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );
}

printItemLabelsByClassCode::~printItemLabelsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void printItemLabelsByClassCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printItemLabelsByClassCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
    _classCode->setId(param.toInt());

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    _classCode->setPattern(param.toString());

  return NoError;
}

void printItemLabelsByClassCode::sPrint()
{
  q.prepare( "SELECT labelform_report_name AS report_name "
             "FROM labelform "
             "WHERE ( (labelform_id=:labelform_id) );");
  q.bindValue(":labelform_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    _warehouse->appendValue(params);
    _classCode->appendValue(params);

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
  else
    QMessageBox::warning( this, tr("Could not locate report"),
                          tr("Could not locate the report definition the form \"%1\"")
                          .arg(_report->currentText()) );
}

