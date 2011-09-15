/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlError>

#include "financialReportNotes.h"


financialReportNotes::financialReportNotes(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _periodid = -1;
  _flheadid = -1;
  _flnotesid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

financialReportNotes::~financialReportNotes()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialReportNotes::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialReportNotes::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("period_id", &valid);
  if (valid)
    _periodid = param.toInt();

  param = pParams.value("flhead_id", &valid);
  if (valid)
    _flheadid = param.toInt();

  populate();

  return NoError;
}

void financialReportNotes::sSave()
{
  XSqlQuery qry;
  if (_flnotesid == -1)
    qry.prepare("INSERT INTO flnotes "
              "(flnotes_period_id, flnotes_flhead_id, flnotes_notes) "
              "VALUES "
              "(:flnotes_period_id, :flnotes_flhead_id, :flnotes_notes) ");
  else
    qry.prepare( "UPDATE flnotes "
               "SET flnotes_notes=:flnotes_notes "
               "WHERE flnotes_id=:flnotes_id;");

  qry.bindValue(":flnotes_id", _flnotesid);
  qry.bindValue(":flnotes_period_id", _periodid);
  qry.bindValue(":flnotes_flhead_id", _flheadid);
  qry.bindValue(":flnotes_notes", _notes->toPlainText());
  qry.exec();
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                .arg(__FILE__)
                .arg(__LINE__) );
    return;
  }

  accept();
}

void financialReportNotes::populate()
{
  XSqlQuery qry;
  qry.prepare( "SELECT flhead_name, "
               "CASE WHEN (length(period_name) > 0) THEN "
               "  period_name "
               "ELSE "
               "  (formatDate(period_start) || '-' || formatDate(period_end)) "
               "END AS period_name "
               "FROM flhead, period "
               "WHERE ((flhead_id=:flhead_id) "
               " AND (period_id=:period_id));" );
  qry.bindValue(":flhead_id", _flheadid);
  qry.bindValue(":period_id", _periodid);

  qry.exec();
  if (qry.first())
  {
    _layout->setText(qry.value("flhead_name").toString());
    _period->setText(qry.value("period_name").toString());
  }

  qry.prepare( "SELECT flnotes_id, flnotes_notes "
               "FROM flnotes "
               "WHERE ((flnotes_flhead_id=:flhead_id) "
               " AND (flnotes_period_id=:period_id));" );
  qry.bindValue(":flhead_id", _flheadid);
  qry.bindValue(":period_id", _periodid);
  qry.exec();
  if (qry.first())
  {
    _flnotesid = qry.value("flnotes_id").toInt();
    _notes->setText(qry.value("flnotes_notes").toString());
  }
}

