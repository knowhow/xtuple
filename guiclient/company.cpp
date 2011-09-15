/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "company.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

#include <dbtools.h>

#include "login2.h"
#include "currcluster.h"

#define DEBUG false

company::company(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_extDB,     SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extPort,   SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extServer, SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extTest,   SIGNAL(clicked()), this, SLOT(sTest()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  connect(_currency, SIGNAL(newID(int)), this, SLOT(sCurrencyChanged()));

  _number->setMaxLength(_metrics->value("GLCompanySize").toInt());
  _cachedNumber = "";
  _cachedCurrid = CurrCluster::baseId();

  _external->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));
  _authGroup->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));
  _currency->setId(CurrCluster::baseId());
  _unrlzgainloss->setType(GLCluster::cRevenue | GLCluster::cExpense);
  _unrlzgainloss->setShowExternal(true);
  _unrlzgainloss->setIgnoreCompany(true);
  _yearend->setShowExternal(true);
  _yearend->setType(GLCluster::cEquity);
  _yearend->setIgnoreCompany(true);
  _gainloss->setType(GLCluster::cExpense);
  _gainloss->setShowExternal(true);
  _gainloss->setIgnoreCompany(true);
  _discrepancy->setType(GLCluster::cExpense);
  _discrepancy->setShowExternal(true);
  _discrepancy->setIgnoreCompany(true);
}

company::~company()
{
  // no need to delete child widgets, Qt does it all for us
}

void company::languageChange()
{
  retranslateUi(this);
}

enum SetResponse company::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("company_id", &valid);
  if (valid)
  {
    _companyid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      
      _buttonBox->button(QDialogButtonBox::Save)->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _number->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _external->setEnabled(FALSE);
      _authGroup->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }
  return NoError;
}

void company::sSave()
{
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Company"),
                            tr("You must enter a valid Number.") );
      _number->setFocus();
      return;
  }
  
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _external->isChecked() && _extServer->text().isEmpty(),
      tr("<p>You must enter a Server if this is an external Company."),
      _extServer
    },
    { _external->isChecked() && _extPort->value() == 0,
      tr("<p>You must enter a Port if this is an external Company."),
      _extPort
    },
    { _external->isChecked() && _extDB->text().isEmpty(),
      tr("<p>You must enter a Database if this is an external Company."),
      _extDB
    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Company"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  q.prepare("SELECT company_id"
            "  FROM company"
            " WHERE((company_id != :company_id)"
            "   AND (company_number=:company_number))");
  q.bindValue(":company_id",       _companyid);
  q.bindValue(":company_number",   _number->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Company Number"),
      tr("A Company Number already exists for the one specified.") );
    _number->setFocus();
    return;
  }

  if (_mode != cNew)
  {
    if (_yearend->isValid() &&
        _companyid != _yearend->companyId())
    {
      QMessageBox::critical(this, tr("Company Account Mismatch"),
                            tr("The Retained Earnings Account must belong to this Company.") );
      _yearend->setFocus();
      return;
    }

    if (_gainloss->isValid() &&
        _companyid != _gainloss->companyId())
    {
      QMessageBox::critical(this, tr("Company Account Mismatch"),
                            tr("The Currency Gain/Loss Account must belong to this Company.") );
      _gainloss->setFocus();
      return;
    }

    if (_discrepancy->isValid() &&
        _companyid != _discrepancy->companyId())
    {
      QMessageBox::critical(this, tr("Company Account Mismatch"),
                            tr("The G/L Discrepancy Account must belong to this Company.") );
      _discrepancy->setFocus();
      return;
    }

    if (_unrlzgainloss->isValid() &&
        _companyid != _unrlzgainloss->companyId())
    {
      QMessageBox::critical(this, tr("Company Account Mismatch"),
                            tr("The Unrealized Currency Gain/Loss Account must belong to this Company.") );
      _unrlzgainloss->setFocus();
      return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('company_company_id_seq') AS company_id;");
    if (q.first())
      _companyid = q.value("company_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    q.prepare( "INSERT INTO company "
               "( company_id, company_number, company_descrip,"
               "  company_external, company_server, company_port,"
               "  company_database, company_curr_id, company_yearend_accnt_id, "
               "  company_gainloss_accnt_id, company_dscrp_accnt_id, "
               "  company_unrlzgainloss_accnt_id) "
               "VALUES "
               "( :company_id, :company_number, :company_descrip,"
               "  :company_external, :company_server, :company_port, "
               "  :company_database, :company_curr_id, :company_yearend_accnt_id, "
               "  :company_gainloss_accnt_id, :company_dscrp_accnt_id, "
               "  :company_unrlzgainloss_accnt_id);" );
  }
  else if (_mode == cEdit)
  {
    if (_number->text() != _cachedNumber &&
        QMessageBox::question(this, tr("Change All Accounts?"),
                          _external->isChecked() ?
                              tr("<p>The old Company Number %1 might be used "
                                 "by existing Accounts, including Accounts in "
                                 "the %2 child database. Would you like to "
                                 "change all accounts in the current database "
                                 "that use it to Company Number %3?<p>If you "
                                 "answer 'No' then either Cancel or change the "
                                 "Number back to %4 and Save again. If you "
                                 "answer 'Yes' then change the Company Number "
                                 "in the child database as well.")
                                .arg(_cachedNumber)
                                .arg(_extDB->text())
                                .arg(_number->text())
                                .arg(_cachedNumber)
                            :
                              tr("<p>The old Company Number %1 might be used "
                                 "by existing Accounts. Would you like to "
                                 "change all accounts that use it to Company "
                                 "Number %2?<p>If you answer 'No' then either "
                                 "Cancel or change "
                                 "the Number back to %3 and Save again.")
                                .arg(_cachedNumber)
                                .arg(_number->text())
                                .arg(_cachedNumber),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    q.prepare( "UPDATE company "
               "SET company_number=:company_number,"
               "    company_descrip=:company_descrip,"
               "    company_external=:company_external,"
               "    company_server=:company_server,"
               "    company_port=:company_port,"
               "    company_database=:company_database, "
               "    company_curr_id=:company_curr_id, "
               "    company_yearend_accnt_id=:company_yearend_accnt_id, "
               "    company_gainloss_accnt_id=:company_gainloss_accnt_id, "
               "    company_dscrp_accnt_id=:company_dscrp_accnt_id, "
               "    company_unrlzgainloss_accnt_id=:company_unrlzgainloss_accnt_id "
               "WHERE (company_id=:company_id);" );
  }
  
  q.bindValue(":company_id",       _companyid);
  q.bindValue(":company_number",   _number->text());
  q.bindValue(":company_descrip",  _descrip->toPlainText());
  q.bindValue(":company_external", _external->isChecked());
  q.bindValue(":company_server",   _extServer->text());
  q.bindValue(":company_port",     _extPort->cleanText());
  q.bindValue(":company_database", _extDB->text());
  if (_gainloss->isValid())
    q.bindValue(":company_gainloss_accnt_id", _gainloss->id());
  if (_discrepancy->isValid())
    q.bindValue(":company_dscrp_accnt_id", _discrepancy->id());
  if (_yearend->isValid())
    q.bindValue(":company_yearend_accnt_id", _yearend->id());
  if (_external->isChecked())
  {
    q.bindValue(":company_curr_id", _currency->id());
    if (_unrlzgainloss->isValid())
      q.bindValue(":company_unrlzgainloss_accnt_id", _unrlzgainloss->id());
  }
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if ((!_yearend->isValid()) ||
     (!_gainloss->isValid()) ||
     (!_discrepancy->isValid()) ||
     (_external->isChecked() &&
      _currency->id() != CurrCluster::baseId() &&
      !_unrlzgainloss->isValid()))
  {
    QMessageBox::warning( this, tr("Accounts Required"),
                          tr("You will need to return to this window to set "
                             "required Accounts before you can use Accounts "
                             "for this company in the system.") );
  }
  
  done(_companyid);
}

void company::populate()
{
  q.prepare( "SELECT * "
             "FROM company "
             "WHERE (company_id=:company_id);" );
  q.bindValue(":company_id", _companyid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("company_number").toString());
    _descrip->setText(q.value("company_descrip").toString());
    _external->setChecked(q.value("company_external").toBool());
    _extServer->setText(q.value("company_server").toString());
    _extPort->setValue(q.value("company_port").toInt());
    _extDB->setText(q.value("company_database").toString());
    _yearend->setId(q.value("company_yearend_accnt_id").toInt());
    _gainloss->setId(q.value("company_gainloss_accnt_id").toInt());
    _discrepancy->setId(q.value("company_dscrp_accnt_id").toInt());
    if (_external->isChecked())
    {
      _cachedCurrid = q.value("company_curr_id").toInt();
      _currency->setId(q.value("company_curr_id").toInt());
      _unrlzgainloss->setId(q.value("company_unrlzgainloss_accnt_id").toInt());
    }

    _cachedNumber = q.value("company_number").toString();

    q.prepare("SELECT COUNT(*) AS result "
              "FROM accnt "
              "WHERE (accnt_company=:number);");
    q.bindValue(":number", _cachedNumber);
    q.exec();
    if (q.first())
      _external->setEnabled(q.value("result").toInt() == 0);
    else if (q.lastError().type() != QSqlError::NoError)
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
  sHandleTest();
}

void company::sHandleTest()
{
  _extTest->setEnabled(! _extServer->text().isEmpty() &&
                       _extPort->value() != 0 &&
                       ! _extDB->text().isEmpty());
}

void company::sTest()
{
  if (DEBUG)
    qDebug("company::sTest()");

  QString dbURL;
  QString protocol = "psql";
  QString host = _extServer->text();
  QString db   = _extDB->text();
  QString port = _extPort->cleanText();

  buildDatabaseURL(dbURL, protocol, host, db, port);
  if (DEBUG)
    qDebug("company::sTest() dbURL before login2 = %s", qPrintable(dbURL));

  ParameterList params;
  params.append("databaseURL", dbURL);
  params.append("multipleConnections");

  login2 newdlg(this, "testLogin", false);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Rejected)
    return;

  dbURL = newdlg._databaseURL;
  if (DEBUG)
    qDebug("company::sTest() dbURL after login2 = %s", qPrintable(dbURL));
  parseDatabaseURL(dbURL, protocol, host, db, port);

  QSqlDatabase testDB = QSqlDatabase::addDatabase("QPSQL7", db);
  testDB.setHostName(host);
  testDB.setDatabaseName(db);
  testDB.setUserName(newdlg.username());
  testDB.setPassword(newdlg.password());
  testDB.setPort(port.toInt());
  if (testDB.open())
  {
    if (DEBUG)
      qDebug("company::sTest() opened testDB!");

    XSqlQuery rmq(testDB);
    rmq.prepare("SELECT fetchMetricText('ServerVersion') AS result;");
    rmq.exec();
    if (rmq.first())
    {
      if (rmq.value("result").toString() != _metrics->value("ServerVersion"))
      {
        QMessageBox::warning(this, tr("Versions Incompatible"),
                             tr("<p>The version of the child database is not "
                                "the same as the version of the parent "
                                "database (%1 vs. %2). The data cannot safely "
                                "be synchronized.")
                             .arg(rmq.value("result").toString())
                             .arg(_metrics->value("ServerVersion")));
        return;
      }
    }
    else if (rmq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    rmq.exec("SELECT * FROM curr_symbol WHERE curr_base;");
    if (_external->isChecked())
    {
      q.prepare("SELECT * FROM curr_symbol WHERE curr_id=:curr_id;");
      q.bindValue(":curr_id", _currency->id());
      q.exec();
    }
    else
      q.exec("SELECT * FROM curr_symbol WHERE curr_base;");

    if (q.first() && rmq.first())
    {
      if (rmq.value("curr_symbol").toString() != q.value("curr_symbol").toString() &&
          rmq.value("curr_abbr").toString() != q.value("curr_abbr").toString())
      {
        QMessageBox::warning(this, tr("Currencies Incompatible"),
                             tr("<p>The currency of the child database does "
                                "not appear to match the selected currency for "
                                "the company (%1 %2 %3 vs. %4 %5 %6). The data may "
                                "not synchronize properly.")
                             .arg(rmq.value("curr_name").toString())
                             .arg(rmq.value("curr_symbol").toString())
                             .arg(rmq.value("curr_abbr").toString())
                             .arg(q.value("curr_name").toString())
                             .arg(q.value("curr_symbol").toString())
                             .arg(q.value("curr_abbr").toString()));
        return;
      }
    }
    else if (rmq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (!rmq.first())
    {
      QMessageBox::warning(this, tr("No Base Currency"),
                           tr("<p>The child database does not appear to have "
                              "a base currency defined. The data cannot safely "
                              "be synchronized."));
      return;
    }
    else if (!q.first())
    {
      QMessageBox::warning(this, tr("No Base Currency"),
                           tr("<p>The parent database does not appear to have "
                              "a base currency defined. The data cannot safely "
                              "be synchronized."));
      return;
    }

    rmq.prepare("SELECT * FROM company WHERE (company_number=:number);");
    rmq.bindValue(":number", _number->text());
    rmq.exec();
    if (rmq.first())
    {
      QMessageBox::warning(this, windowTitle(), tr("Test Successful."));
      return;
    }
    else if (rmq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::warning(this, tr("No Corresponding Company"),
                           tr("<p>The child database does not appear to have "
                              "a Company %1 defined. The data cannot safely "
                              "be synchronized.").arg(_number->text()));
      return;
    }
  }
}

void company::sCurrencyChanged()
{
  if (!_external->isChecked())
    return;

  if (_currency->id() != _cachedCurrid)
  {
    XSqlQuery qry;
    qry.prepare("SELECT count(trialbal_id) "
                "FROM trialbal "
                " JOIN accnt ON (trialbal_accnt_id=accnt_id) "
                "WHERE (accnt_company=:company_number);");
    qry.bindValue(":company_number", _number->text());
    qry.exec();
    qry.first();
    if (qry.value("count").toInt())
    {
      if (QMessageBox::question(this, tr("Delete Imported Data?"),
                                tr("Financial history has already been imported "
                                   "for this company. Changing the currency will delete "
                                   "this data. This action is not reversible.  Are you "
                                   "sure this is what you want to do?"),
                                QMessageBox::Yes,
                                QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
      {
        qry.prepare("DELETE FROM gltranssync "
                    "WHERE (gltranssync_company_id=:company_id); "
                    "DELETE FROM trialbal "
                    "WHERE (trialbal_accnt_id IN ("
                    "  SELECT accnt_id "
                    "  FROM accnt "
                    "    JOIN company ON (accnt_company=company_number) "
                    "  WHERE (company_id=:company_id)));");
        qry.bindValue(":company_id", _companyid);
        qry.exec();
        if (qry.lastError().type() != QSqlError::NoError)
        {
          systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
          return;
        }
      }
      else
        _currency->setId(_cachedCurrid);
    }
  }
  _unrlzgainloss->setEnabled(_currency->id() != CurrCluster::baseId());
}
