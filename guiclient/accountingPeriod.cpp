/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountingPeriod.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "unpostedGLTransactions.h"

accountingPeriod::accountingPeriod(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _year->setType(XComboBox::FiscalYears);
}

accountingPeriod::~accountingPeriod()
{
    // no need to delete child widgets, Qt does it all for us
}

void accountingPeriod::languageChange()
{
    retranslateUi(this);
}

enum SetResponse accountingPeriod::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    _periodid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
      q.exec("SELECT period_id "
             "FROM period "
             "WHERE (period_closed) "
             "LIMIT 1; ");
      if (q.first())
      {
        _startDate->setEnabled(false);
        _year->setEnabled(false);
      }
      
      q.exec("SELECT (LAST(period_end) + 1) AS start_date "
             "FROM (SELECT period_end "
             "      FROM period "
             "      ORDER BY period_end) AS data; ");
      if (q.first())
      {
        _startDate->setDate(q.value("start_date").toDate());
        int pmonth = _startDate->date().month();
        QDate pdate = _startDate->date();
        if(pdate.isValid())
        {
          while (pmonth == _startDate->date().month())
          {
            _endDate->setDate(pdate);
            pdate = pdate.addDays(1);
            pmonth = pdate.month();
          }
        }
        sHandleNumber();
        connect(_year, SIGNAL(newID(int)), this, SLOT(sHandleNumber()));
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _startDate->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _startDate->setEnabled(FALSE);
      _endDate->setEnabled(FALSE);
      _closed->setEnabled(FALSE);
      _frozen->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void accountingPeriod::sHandleNumber()
{
  q.prepare("SELECT COALESCE(MAX(period_number),0) + 1 AS number "
            "FROM period "
            "WHERE (period_yearperiod_id=:yearperiod_id);");
  q.bindValue(":yearperiod_id", _year->id());
  q.exec();
  if (q.first())
    _number->setValue(q.value("number").toInt());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountingPeriod::sSave()
{
  if (_startDate->date() >= _endDate->date())
  {
    QMessageBox::critical( this, tr("Cannot Save Period"),
          tr("The start date must be less than the end date.") );
    return;
  }

  if (_mode == cNew)
  {
    q.prepare("SELECT createAccountingPeriod(:startDate, :endDate, :yearperiod_id, :quarter) AS _period_id;");
    q.bindValue(":startDate", _startDate->date());
    q.bindValue(":endDate", _endDate->date());
    q.bindValue(":yearperiod_id", _year->id());
    q.bindValue(":quarter", _quarter->value());

    q.exec();
    if (q.first())
    {
      _periodid = q.value("_period_id").toInt();
      if (_periodid < 0)
      {
	systemError(this, storedProcErrorLookup("createAccountingPeriod", _periodid),
		    __FILE__, __LINE__);
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
    if ( (_cachedFrozen) && (!_frozen->isChecked()) )
    {
      q.prepare("SELECT thawAccountingPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("thawAccountingPeriod", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (_cachedClosed) && (!_closed->isChecked()) )
    {
      bool reallyOpen = false;

      q.prepare("SELECT COUNT(gltrans_sequence) AS count "
		"FROM gltrans, period "
		"WHERE ( (NOT gltrans_posted) "
		"AND (NOT gltrans_deleted) "
		"AND (gltrans_date BETWEEN period_start AND period_end) "
		"AND (period_id=:period_id) );");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	if (q.value("count").toInt() <= 0)
	  reallyOpen = true;
	else
	{
	  ParameterList params;

	  unpostedGLTransactions newdlg(this, "", true);
	  params.append("period_id", _periodid);
	  newdlg.set(params);

	  reallyOpen = (newdlg.exec() == XDialog::Accepted);
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      if (reallyOpen)
      {
	q.prepare("SELECT openAccountingPeriod(:period_id) AS result;");
	q.bindValue(":period_id", _periodid);
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("openAccountingPeriod", result),
			__FILE__, __LINE__);
	    return;
	  }
	}
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
      else
	return;
    }

    if ( (_cachedStartDate != _startDate->date()) ||
         (_cachedEndDate != _endDate->date()) )
    {
      q.prepare("SELECT changeAccountingPeriodDates(:period_id, :startDate, :endDate) AS result;");
      q.bindValue(":period_id", _periodid);
      q.bindValue(":startDate", _startDate->date());
      q.bindValue(":endDate", _endDate->date());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("changeAccountingPeriodDates", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    if ( (!_cachedFrozen) && (_frozen->isChecked()) )
    {
      q.prepare("SELECT freezeAccountingPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("freezeAccountingPeriod", result),
		      __FILE__, __LINE__);
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
      q.prepare("SELECT closeAccountingPeriod(:period_id) AS result;");
      q.bindValue(":period_id", _periodid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("closeAccountingPeriod", result),
		      __FILE__, __LINE__);
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

  if( (_cachedName != _name->text()) || (_cachedYearPeriodId != _year->id()) || (_cachedQuarter != _quarter->value()) )
  {
    q.prepare("SELECT yearperiod_id "
              "FROM yearperiod "
              "WHERE ((yearperiod_id=:yearperiod_id) "
              "AND (:startDate>=yearperiod_start) "
              " AND (:endDate<=yearperiod_end)); ");
    q.bindValue(":yearperiod_id", _year->id());
    q.bindValue(":startDate", _startDate->date());
    q.bindValue(":endDate", _endDate->date());
    q.exec();
    if (q.first())
    {
      q.prepare("UPDATE period SET period_name=:period_name,"
		"                  period_yearperiod_id=:yearperiod_id,"
		"                  period_quarter=:quarter"
                " WHERE (period_id=:period_id); ");
      q.bindValue(":period_id", _periodid);
      q.bindValue(":period_name", _name->text());
      q.bindValue(":yearperiod_id", _year->id());
      q.bindValue(":quarter",	_quarter->value());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical( this, tr("Cannot Save Period"),
            tr("Period dates are outside the selected Fiscal Year.") );
      return;
    }
  }

  done(_periodid);
}

void accountingPeriod::populate()
{
  q.exec( "SELECT FIRST(period_id) AS first_period_id, "
            "  LAST(period_id) AS last_period_id "
            "FROM (SELECT period_id FROM period "
            "      ORDER BY period_start) AS data; ");
  if (q.first())
  {
    if (q.value("first_period_id").toInt() != _periodid)
      _startDate->setEnabled(false);
    if (q.value("last_period_id").toInt() != _periodid)
      _endDate->setEnabled(false);    
  }
  
  q.prepare( "SELECT period_start, period_end, period_closed, period_freeze,"
             "       period_name, period_yearperiod_id, period_quarter, "
             "       period_number, "
             "       (COUNT(trialbal_id) > 0) AS hasTrialBal "
             "FROM period "
             "  LEFT OUTER JOIN trialbal ON (period_id = trialbal_period_id) "
             "WHERE (period_id=:period_id) "
             "GROUP BY period_start, period_end, period_closed, period_freeze,"
             "         period_number, period_name, period_yearperiod_id, period_quarter;" );
  q.bindValue(":period_id", _periodid);
  q.exec();
  if (q.first())
  {
    if (q.value("hasTrialBal").toBool())
    {
      _startDate->setEnabled(false);
      _endDate->setEnabled(false);
      _year->setEnabled(false);
    }
    _cachedStartDate = q.value("period_start").toDate();
    _cachedEndDate = q.value("period_end").toDate();
    _cachedClosed = q.value("period_closed").toBool();
    _cachedFrozen = q.value("period_freeze").toBool();
    _cachedName = q.value("period_name").toString();
    _cachedYearPeriodId = q.value("period_yearperiod_id").toInt();
    _cachedQuarter = q.value("period_quarter").toInt();

    _startDate->setDate(q.value("period_start").toDate());
    _endDate->setDate(q.value("period_end").toDate());
    _closed->setChecked(q.value("period_closed").toBool());
    _frozen->setChecked(q.value("period_freeze").toBool());
    _name->setText(q.value("period_name").toString());
    _year->setId(q.value("period_yearperiod_id").toInt());
    _number->setValue(q.value("period_number").toInt());
    _quarter->setValue(q.value("period_quarter").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
