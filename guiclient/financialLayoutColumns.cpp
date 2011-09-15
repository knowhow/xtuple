/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutColumns.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QWorkspace>

#include <openreports.h>
#include <reporthandler.h>

financialLayoutColumns::financialLayoutColumns(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_month, SIGNAL(toggled(bool)), this, SLOT(sToggleMonth()));
  connect(_quarter, SIGNAL(toggled(bool)), this, SLOT(sToggleQuarter()));
  connect(_year, SIGNAL(toggled(bool)), this, SLOT(sToggleYear()));
  connect(_fullmonth, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_fullquarter, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_fullyear, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_yeartodate, SIGNAL(toggled(bool)), this, SLOT(sTogglePrior()));
  connect(_fullyear, SIGNAL(toggled(bool)), this, SLOT(sToggleYearToDate()));
  connect(_yeartodate, SIGNAL(toggled(bool)), this, SLOT(sToggleFullYear()));

  languageChange();
  
  //Hide Edit Button until we can figure out how to implement this
  _edit->setHidden(true);
}

financialLayoutColumns::~financialLayoutColumns()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialLayoutColumns::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutColumns::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
  {
    _flheadid = param.toInt();
  }

  param = pParams.value("flcol_id", &valid);
  if (valid)
  {
    _flcolid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
    _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
    _mode = cEdit;
    _buttonBox->setFocus();
    populate();
    }
    else if (param.toString() == "view")
    {
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _report->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
      _selected->setEnabled(FALSE);
      _prior->setEnabled(FALSE);
      _budget->setEnabled(FALSE);
      _showdb->setEnabled(FALSE);
    }
  }
  
  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "balance")
    {
      _month->setChecked(TRUE);
      _month->setEnabled(FALSE);
      _fullmonth->setEnabled(TRUE);
      _fullquarter->setEnabled(TRUE);
      _fullyear->setEnabled(TRUE);
      _yeartodate->setEnabled(TRUE);
      _quarter->setHidden(TRUE);
      _year->setHidden(TRUE);
      _yeartodate->setHidden(TRUE);
      _fullmonth->setText(tr("Month End"));
      _fullquarter->setText(tr("Quarter End"));
      _fullyear->setText(tr("Year End"));
    }
    if (param.toString() != "cash")
      _showdb->setHidden(TRUE);
  }
  
  return NoError;
}

void financialLayoutColumns::sSave()
{ 
  QString sql;

  if ((!_month->isChecked()) && (!_quarter->isChecked()) && (!_year->isChecked()))
  {
    QMessageBox::critical( this, tr("Cannot Save settings"),
              tr("<p>At least one of Month, Date or Year must be selected.") );
    return;
  }
      
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('flcol_flcol_id_seq') AS flcol_id;");
    if (q.first())
      _flcolid = q.value("flcol_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
      
    sql = ( "INSERT INTO flcol "
        "(flcol_id,flcol_flhead_id,flcol_name,flcol_descrip,"
        "flcol_report_id,flcol_month,flcol_quarter,flcol_year,"
        "flcol_showdb,flcol_prcnt,flcol_priortype,flcol_priormonth,"
        "flcol_priorquarter,flcol_prioryear,flcol_priorprcnt,"
        "flcol_priordiff,flcol_priordiffprcnt,flcol_budget,"
        "flcol_budgetprcnt,flcol_budgetdiff,flcol_budgetdiffprcnt) "
        "VALUES "
         "(:flcol_id,:flcol_flhead_id,:flcol_name,:flcol_descrip,"
        ":flcol_report_id,:flcol_month,:flcol_quarter,:flcol_year,"
        ":flcol_showdb,:flcol_prcnt,:flcol_priortype,:flcol_priormonth,"
        ":flcol_priorquarter,:flcol_prioryear,:flcol_priorprcnt,"
        ":flcol_priordiff,:flcol_priordiffprcnt,:flcol_budget,"
        ":flcol_budgetprcnt,:flcol_budgetdiff,:flcol_budgetdiffprcnt);" );
  }
  else if (_mode == cEdit)
  {
    sql =  ("UPDATE flcol "
        "SET flcol_name=:flcol_name,flcol_descrip=:flcol_descrip,"
        "flcol_report_id=:flcol_report_id,flcol_month=:flcol_month,"
        "flcol_quarter=:flcol_quarter,flcol_year=:flcol_year,"
        "flcol_showdb=:flcol_showdb,flcol_prcnt=:flcol_prcnt,"
        "flcol_priortype=:flcol_priortype,flcol_priormonth=:flcol_priormonth,"
        "flcol_priorquarter=:flcol_priorquarter,"
        "flcol_prioryear=:flcol_prioryear,flcol_priorprcnt=:flcol_priorprcnt,"
        "flcol_priordiff=:flcol_priordiff,flcol_priordiffprcnt=:flcol_priordiffprcnt,"
        "flcol_budget=:flcol_budget,flcol_budgetprcnt=:flcol_budgetprcnt,"
        "flcol_budgetdiff=:flcol_budgetdiff,flcol_budgetdiffprcnt=:flcol_budgetdiffprcnt"
        " WHERE (flcol_id=:flcol_id);");
  }

  q.prepare(sql);
     
  q.bindValue(":flcol_id", _flcolid);
  q.bindValue(":flcol_flhead_id", _flheadid);
  q.bindValue(":flcol_name", _name->text());
  q.bindValue(":flcol_descrip", _descrip->text());
  q.bindValue(":flcol_report_id", _report->id());
  q.bindValue(":flcol_month", _month->isChecked());
  q.bindValue(":flcol_quarter", _quarter->isChecked());
  q.bindValue(":flcol_year", _year->isChecked());
  q.bindValue(":flcol_showdb", _showdb->isChecked());
  q.bindValue(":flcol_prcnt", _prcnt->isChecked());
  if (_priorperiod->isChecked())
    q.bindValue(":flcol_priortype", "P");
  else
    q.bindValue(":flcol_priortype", "Y");
  q.bindValue(":flcol_priormonth", _fullmonth->isChecked());
  q.bindValue(":flcol_priorquarter", _fullquarter->isChecked());
  if (_fullyear->isChecked())
    q.bindValue(":flcol_prioryear", "F");
  else if (_yeartodate->isChecked())
    q.bindValue(":flcol_prioryear", "D");
  else
    q.bindValue(":flcol_prioryear", "N");
  q.bindValue(":flcol_priorprcnt", _priorprcnt->isChecked());
  q.bindValue(":flcol_priordiff", _priordiff->isChecked());
  q.bindValue(":flcol_priordiffprcnt", _priordiffprcnt->isChecked());
  q.bindValue(":flcol_budget", _budget->isChecked());
  if (_budget->isChecked())
  {
    q.bindValue(":flcol_budgetprcnt", _budgetprcnt->isChecked());
    q.bindValue(":flcol_budgetdiff", _budgetdiff->isChecked());
    q.bindValue(":flcol_budgetdiffprcnt", _budgetdiffprcnt->isChecked());
  }
  else
  {
      q.bindValue(":flcol_budgetprcnt", false);
    q.bindValue(":flcol_budgetdiff", false);
    q.bindValue(":flcol_budgetdiffprcnt", false);
  }
    
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_flcolid);
}

void financialLayoutColumns::populate()
{
  q.prepare( "SELECT * "
             "FROM flcol "
             "WHERE (flcol_id=:flcol_id);" );
  q.bindValue(":flcol_id", _flcolid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("flcol_name"));
    _descrip->setText(q.value("flcol_descrip"));
    _report->setId(q.value("flcol_report_id").toInt());
    _month->setChecked(q.value("flcol_month").toBool());
    _quarter->setChecked(q.value("flcol_quarter").toBool());
    _year->setChecked(q.value("flcol_year").toBool());
    _showdb->setChecked(q.value("flcol_showdb").toBool());
    _prcnt->setChecked(q.value("flcol_prcnt").toBool());
  if (q.value("flcol_priortype").toString() == "P")
        _priorperiod->setChecked(true);
  else
        _prioryear->setChecked(true);
    _fullmonth->setChecked(q.value("flcol_priormonth").toBool());
    _fullquarter->setChecked(q.value("flcol_priorquarter").toBool());
    if (q.value("flcol_prioryear").toString() == "F")
    {
        _fullyear->setChecked(true);
        _yeartodate->setChecked(false);
    }
    else if (q.value("flcol_prioryear").toString() == "D")
    {
        _yeartodate->setChecked(true);
        _fullyear->setChecked(false);
    }
    else
    {
        _fullyear->setChecked(false);
        _yeartodate->setChecked(false);
    }
    _priorprcnt->setChecked(q.value("flcol_priorprcnt").toBool());
    _priordiff->setChecked(q.value("flcol_priordiff").toBool());
    _priordiffprcnt->setChecked(q.value("flcol_priordiffprcnt").toBool());
    _budget->setChecked(q.value("flcol_budget").toBool());
    _budgetprcnt->setChecked(q.value("flcol_budgetprcnt").toBool());  
    _budgetdiff->setChecked(q.value("flcol_budgetdiff").toBool());
    _budgetdiffprcnt->setChecked(q.value("flcol_budgetdiffprcnt").toBool());  
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void financialLayoutColumns::sToggleMonth()
{
  if (!_month->isChecked())
  {
    _fullmonth->setChecked(FALSE);
    _fullmonth->setEnabled(FALSE);
  }
  else
    _fullmonth->setEnabled(TRUE);
}

void financialLayoutColumns::sToggleQuarter()
{
  if (!_quarter->isChecked())
  {
    _fullquarter->setChecked(FALSE);
    _fullquarter->setEnabled(FALSE);
  }
  else
    _fullquarter->setEnabled(TRUE);
}

void financialLayoutColumns::sToggleYear()
{
  if (!_year->isChecked())
  {
    _fullyear->setChecked(FALSE);
    _yeartodate->setChecked(FALSE);
    _fullyear->setEnabled(FALSE);
    _yeartodate->setEnabled(FALSE);
  }
  else
  {
    _fullyear->setEnabled(TRUE);
    _yeartodate->setEnabled(TRUE);
  }
}

void financialLayoutColumns::sTogglePrior()
{
  if ((_fullmonth->isChecked()) || (_fullquarter->isChecked()) || (_fullyear->isChecked()) || (_yeartodate->isChecked()))
  {
    _priorprcnt->setEnabled(TRUE);
    _priordiff->setEnabled(TRUE);
    _priordiffprcnt->setEnabled(TRUE);
  }
  else
  {
    _priorprcnt->setChecked(FALSE);
    _priordiff->setChecked(FALSE);
    _priordiffprcnt->setChecked(FALSE);  
    _priorprcnt->setEnabled(FALSE);
    _priordiff->setEnabled(FALSE);
    _priordiffprcnt->setEnabled(FALSE);    
  }
}

void financialLayoutColumns::sToggleYearToDate()
{
  if (_fullyear->isChecked())
    _yeartodate->setChecked(false);
}

void financialLayoutColumns::sToggleFullYear()
{
  if (_yeartodate->isChecked())
    _fullyear->setChecked(false);
}

void financialLayoutColumns::sEdit()
{
  if (!omfgThis->_reportHandler)
  {
    omfgThis->_reportHandler = new ReportHandler(omfgThis, "report handler");
    omfgThis->_reportHandler->setParentWindow(omfgThis->workspace());
    omfgThis->_reportHandler->setAllowDBConnect(FALSE);
    omfgThis->_reportHandler->setPlaceMenusOnWindows(TRUE);
    omfgThis->_reportHandler->setPlaceToolbarsOnWindows(TRUE);
    connect(omfgThis->_reportHandler, SIGNAL(reportsChanged(int, bool)), omfgThis, SLOT(sReportsChanged(int, bool)));
  }

  q.prepare( "SELECT report_name, report_grade, report_source "
             "  FROM report "
             " WHERE (report_id=:report_id); " );
  q.bindValue(":report_id", _report->id());
  q.exec();
  if (q.first())
  {
    QDomDocument doc;
    QString errorMessage;
    int errorLine;
    int errorColumn;

    if (doc.setContent(q.value("report_source").toString(), &errorMessage, &errorLine, &errorColumn))
      omfgThis->_reportHandler->fileOpen(doc, q.value("report_name").toString(), q.value("report_grade").toInt());
    else
      QMessageBox::warning( this, tr("Error Loading Report"),
                            tr( "ERROR parsing content:\n"
                                "\t%1 (Line %2 Column %3)" )
                            .arg(errorMessage)
                            .arg(errorLine)
                            .arg(errorColumn) );
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
