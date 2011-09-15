/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountingYearPeriod.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

accountingYearPeriod::accountingYearPeriod(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

accountingYearPeriod::~accountingYearPeriod()
{
    // no need to delete child widgets, Qt does it all for us
}

void accountingYearPeriod::languageChange()
{
    retranslateUi(this);
}

enum SetResponse accountingYearPeriod::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("yearperiod_id", &valid);
  if (valid)
  {
    _periodid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    XSqlQuery check;
    check.exec("SELECT yearperiod_id "
               "FROM yearperiod "
               "WHERE (yearperiod_closed) "
               "LIMIT 1; ");
    if (check.first())
    {
      _startDate->setEnabled(false);
      _endDate->setFocus();
    }
    else
      _startDate->setFocus();

    if (param.toString() == "new")
    {
      _mode = cNew;

      check.exec("SELECT yearperiod_end + 1 AS start_date "
                 "FROM yearperiod "
                 "ORDER BY yearperiod_end DESC "
                 "LIMIT 1; ");
      if (check.first())
      {
        _startDate->setDate(check.value("start_date").toDate());
        _endDate->setDate(check.value("start_date").toDate().addYears(1).addDays(-1));
      }

      _startDate->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _startDate->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _startDate->setEnabled(FALSE);
      _endDate->setEnabled(FALSE);
      _closed->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void accountingYearPeriod::sSave()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT createAccountingYearPeriod(:startDate, :endDate) AS _period_id;");
    q.bindValue(":startDate", _startDate->date());
    q.bindValue(":endDate", _endDate->date());
    q.exec();
    if (q.first())
    {
      _periodid = q.value("_period_id").toInt();
      if (_periodid < 0)
      {
	systemError(this, storedProcErrorLookup("createAccountingYearPeriod", _periodid), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEdit)
  {

    if ( (_cachedClosed) && (!_closed->isChecked()) )
    {
      q.prepare("SELECT openAccountingYearPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("openAccountingYearPeriod", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (_cachedStartDate != _startDate->date()) ||
         (_cachedEndDate != _endDate->date()) )
    {
      q.prepare("SELECT changeAccountingYearPeriodDates(:period_id, :startDate, :endDate) AS result;");
      q.bindValue(":period_id", _periodid);
      q.bindValue(":startDate", _startDate->date());
      q.bindValue(":endDate", _endDate->date());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("createAccountingYearPeriod", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (!_cachedClosed) && (_closed->isChecked()) )
    {
      q.prepare("SELECT closeAccountingYearPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("closeAccountingYearPeriod", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }

  done(_periodid);
}

void accountingYearPeriod::populate()
{
  q.exec( "SELECT FIRST(yearperiod_id) AS first_yearperiod_id, "
            "  LAST(yearperiod_id) AS last_yearperiod_id "
            "FROM (SELECT yearperiod_id FROM yearperiod "
            "      ORDER BY yearperiod_start) AS data; ");
  if (q.first())
  {
    if (q.value("first_yearperiod_id").toInt() != _periodid)
      _startDate->setEnabled(false);
    if (q.value("last_yearperiod_id").toInt() != _periodid)
      _endDate->setEnabled(false);
  }

  q.prepare( "SELECT yearperiod_start, yearperiod_end, yearperiod_closed "
             "FROM yearperiod "
             "WHERE (yearperiod_id=:period_id);" );
  q.bindValue(":period_id", _periodid);
  q.exec();
  if (q.first())
  {
    _cachedStartDate = q.value("yearperiod_start").toDate();
    _cachedEndDate = q.value("yearperiod_end").toDate();
    _cachedClosed = q.value("yearperiod_closed").toBool();

    _startDate->setDate(q.value("yearperiod_start").toDate());
    _endDate->setDate(q.value("yearperiod_end").toDate());
    _closed->setChecked(q.value("yearperiod_closed").toBool());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
