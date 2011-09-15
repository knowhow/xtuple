/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printLabelsByOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

printLabelsByOrder::printLabelsByOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_labelFrom, SIGNAL(valueChanged(int)), this, SLOT(sSetToMin(int)));
  connect(_print,	      SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = FALSE;

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );

  _order->setAllowedTypes(OrderLineEdit::Purchase |
                          OrderLineEdit::Return   |
                          OrderLineEdit::Transfer);
  _order->setAllowedStatuses(OrderLineEdit::Unposted |
                             OrderLineEdit::Open);
}

printLabelsByOrder::~printLabelsByOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void printLabelsByOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printLabelsByOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("ordertype", &valid);
  if (valid)
  {
    _order->setAllowedType(param.toString());
    _print->setFocus();
  }

  param = pParams.value("orderid", &valid);
  if (valid)
  {
    _order->setId(param.toInt());
    _print->setFocus();
  }

  return NoError;
}

void printLabelsByOrder::sPrint()
{
  q.prepare( "SELECT labelform_report_name AS report_name "
             "FROM labelform "
             "WHERE ( (labelform_id=:labelform_id) );" );
  q.bindValue(":labelform_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("orderhead_type", _order->type());
    params.append("orderhead_id",   _order->id());

    if (_order->isPO())
      params.append("pohead_id", _order->id());
    else if (_order->isTO())
      params.append("tohead_id", _order->id());
    else if (_order->isRA())
      params.append("rahead_id", _order->id());

    params.append("labelFrom", _labelFrom->value());
    params.append("labelTo", _labelTo->value());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    _order->setId(-1);
    _order->setFocus();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::warning(this, tr("Could not locate report"),
                         tr("<p>Could not locate the report definition for the form \"%1\"")
                         .arg(_report->currentText()) );

  if (_captive)
    accept();
}

void printLabelsByOrder::sSetToMin(int pValue)
{
  _labelTo->setMinimum(pValue);
}
