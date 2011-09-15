/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printLabelsBySo.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

printLabelsBySo::printLabelsBySo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _so->setAllowedTypes(OrderLineEdit::Sales);
  QPushButton* _print = _buttonBox->button(QDialogButtonBox::Ok);
  _print->setEnabled(false);

  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_from, SIGNAL(valueChanged(int)), this, SLOT(sSetToMin(int)));

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );
}

printLabelsBySo::~printLabelsBySo()
{
  // no need to delete child widgets, Qt does it all for us
}

void printLabelsBySo::languageChange()
{
  retranslateUi(this);
}

void printLabelsBySo::sPrint()
{
  q.prepare( "SELECT labelform_report_name AS report_name "
             "FROM labelform "
             "WHERE ( (labelform_id=:labelform_id) );");
  q.bindValue(":labelform_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", _so->id());
    params.append("labelFrom", _from->value());
    params.append("labelTo", _to->value());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    _so->setId(-1);
    _so->setFocus();
  }
  else
    QMessageBox::warning(this, tr("Could not locate report"),
                         tr("Could not locate the report definition the form \"%1\"")
                         .arg(_report->currentText()) );
}

void printLabelsBySo::sSetToMin(int pValue)
{
  _to->setMinimum(pValue);
}
