/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureCRM.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "guiclient.h"
#include "mqlutil.h"

configureCRM::configureCRM(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  _nextInNumber->setValidator(omfgThis->orderVal());
  _nextAcctNumber->setValidator(omfgThis->orderVal());

  q.exec( "SELECT orderseq_number AS innumber "
          "  FROM orderseq"
          " WHERE (orderseq_name='IncidentNumber');" );
  if (q.first())
    _nextInNumber->setText(q.value("innumber"));
    
  q.exec( "SELECT orderseq_number AS acnumber "
          "  FROM orderseq"
          " WHERE (orderseq_name='CRMAccountNumber');" );
  if (q.first())
    _nextAcctNumber->setText(q.value("acnumber"));

  QString metric = _metrics->value("CRMAccountNumberGeneration");
  if (metric == "M")
    _acctGeneration->setCurrentIndex(0);
  else if (metric == "A")
    _acctGeneration->setCurrentIndex(1);
  else if (metric == "O")
    _acctGeneration->setCurrentIndex(2);

    
  _useProjects->setChecked(_metrics->boolean("UseProjects"));
  _autoCreate->setChecked(_metrics->boolean("AutoCreateProjectsForOrders"));
  _opportunityChangeLog->setChecked(_metrics->boolean("OpportunityChangeLog"));

  if (! _metrics->value("DefaultAddressCountry").isEmpty())
    _country->setText(_metrics->value("DefaultAddressCountry"));

  _strictCountries->setChecked(_metrics->boolean("StrictAddressCountry"));
  _strictCountries->setEnabled(! _strictCountries->isChecked());
  if (! _strictCountries->isChecked())
  {
    connect(_strictCountries, SIGNAL(toggled(bool)), this, SLOT(sStrictCountryChanged(bool)));
  }

  _incidentsPublicShow->setChecked(_metrics->boolean("IncidentsPublicPrivate"));
  _incidentsPublicDefault->setChecked(_metrics->boolean("IncidentPublicDefault"));

  if (_metrics->boolean("EnableBatchManager"))
  {
    _incdtEmailProfile->populate("SELECT ediprofile_id, ediprofile_name "
                                 "FROM xtbatch.ediprofile "
                                 "WHERE (ediprofile_type='email');");
    _incdtEmailProfile->setId(_metrics->value("CRMIncidentEmailProfile").toInt());
    _incdtCreated->setChecked(_metrics->boolean("CRMIncidentEmailCreated"));
    _incdtAssigned->setChecked(_metrics->boolean("CRMIncidentEmailAssigned"));
    _incdtStatus->setChecked(_metrics->boolean("CRMIncidentEmailStatus"));
    _incdtUpdated->setChecked(_metrics->boolean("CRMIncidentEmailUpdated"));
    _incdtComments->setChecked(_metrics->boolean("CRMIncidentEmailComments"));
  }
  else
  {
    _incdtEmailProfileLit->hide();
    _incdtEmailProfile->hide();
    _incdtDelGroup->hide();
  }

  q.exec("SELECT * FROM status WHERE (status_type='INCDT') ORDER BY status_seq;");
  q.first();
  _new->setText(q.value("status_color"));
  q.next();
  _feedback->setText(q.value("status_color"));
  q.next();
  _confirmed->setText(q.value("status_color"));
  q.next();
  _assigned->setText(q.value("status_color"));
  q.next();
  _resolved->setText(q.value("status_color"));
  q.next();
  _closed->setText(q.value("status_color"));

  adjustSize();
}

configureCRM::~configureCRM()
{
    // no need to delete child widgets, Qt does it all for us
}

void configureCRM::languageChange()
{
    retranslateUi(this);
}

bool configureCRM::sSave()
{
  emit saving();

  const char *numberGenerationTypes[] = { "M", "A", "O" };

  XSqlQuery configq;
  configq.prepare( "SELECT setNextIncidentNumber(:innumber);" );
  configq.bindValue(":innumber", _nextInNumber->text().toInt());
  configq.exec();
  
  configq.prepare( "SELECT setNextCRMAccountNumber(:acnumber);" );
  configq.bindValue(":acnumber", _nextAcctNumber->text().toInt());
  configq.exec();

  _metrics->set("CRMAccountNumberGeneration", QString(numberGenerationTypes[_acctGeneration->currentIndex()]));
  
  _metrics->set("UseProjects", _useProjects->isChecked());
  _metrics->set("AutoCreateProjectsForOrders", (_autoCreate->isChecked() && _useProjects->isChecked()));
  _metrics->set("OpportunityChangeLog", _opportunityChangeLog->isChecked());

  if (_country->isValid())
    _metrics->set("DefaultAddressCountry", _country->currentText());
  else
    _metrics->set("DefaultAddressCountry", QString(""));

  _metrics->set("StrictAddressCountry", _strictCountries->isChecked());
  
  _metrics->set("IncidentsPublicPrivate", _incidentsPublicShow->isChecked());
  _metrics->set("IncidentPublicDefault", _incidentsPublicDefault->isChecked());

  if (_metrics->boolean("EnableBatchManager"))
  {
    _metrics->set("CRMIncidentEmailProfile", _incdtEmailProfile->id());
    _metrics->set("CRMIncidentEmailCreated"   , _incdtCreated->isChecked());
    _metrics->set("CRMIncidentEmailAssigned"  , _incdtAssigned->isChecked());
    _metrics->set("CRMIncidentEmailStatus"    , _incdtStatus->isChecked());
    _metrics->set("CRMIncidentEmailUpdated"   , _incdtUpdated->isChecked());
    _metrics->set("CRMIncidentEmailComments"  , _incdtComments->isChecked());
  }

  configq.prepare("UPDATE status SET status_color = :color "
            "WHERE ((status_type='INCDT') "
            " AND (status_code=:code));");
  configq.bindValue(":code", "N");
  configq.bindValue(":color", _new->text());
  configq.exec();
  configq.bindValue(":code", "F");
  configq.bindValue(":color", _feedback->text());
  configq.exec();
  configq.bindValue(":code", "C");
  configq.bindValue(":color", _confirmed->text());
  configq.exec();
  configq.bindValue(":code", "A");
  configq.bindValue(":color", _assigned->text());
  configq.exec();
  configq.bindValue(":code", "R");
  configq.bindValue(":color", _resolved->text());
  configq.exec();
  configq.bindValue(":code", "L");
  configq.bindValue(":color", _closed->text());
  configq.exec();

  return true;
}

/* TODO: introduced option in 3.4.0beta2.
   deprecate it in the future with strict as the standard.
 */
void configureCRM::sStrictCountryChanged(bool p)
{
  if (p)
  {
    bool mqlloaded;
    MetaSQLQuery mql = mqlLoad("crm", "strictcountrycheck", &mqlloaded);
    if (! mqlloaded)
    {
      QMessageBox::critical(this, tr("Query Not Found"),
                            tr("<p>The application could not find the MetaSQL "
                               "query crm-strictcountrycheck."));
      _strictCountries->setChecked(false);
      return;
    }

    ParameterList params;
    params.append("count");
    XSqlQuery activeq = mql.toQuery(params);

    params.append("showAll");
    XSqlQuery allq = mql.toQuery(params);

    if (activeq.first())
    {
      int result = activeq.value("counter").toInt();
      if (result > 0)
      {
        QMessageBox::warning(this, tr("Invalid Countries"),
                             tr("<p>The database contains invalid countries in "
                                "active records, such as addresses and open "
                                "sales orders. Please correct these records "
                                "before turning on strict country checking. "
                                "You may download and install the "
                                "fixCountry.gz package "
                                "to help with this task."));
        _strictCountries->setChecked(false);
        return;
      }
    }
    else if (activeq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, activeq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (allq.first())
    {
      int result = allq.value("counter").toInt();
      if (result > 0)
      {
        QMessageBox::StandardButton answer =
          QMessageBox::question(this, tr("Invalid Countries"),
                                tr("<p>The database contains invalid countries "
                                   "in historical records, such as closed "
                                   "sales orders and posted invoices. If you "
                                   "do not correct these countries before "
                                   "turning on strict country checking, you "
                                   "may lose country values if you open these "
                                   "documents and save them again. You may "
                                   "download and install the fixCountry.gz "
                                   "package to help update your records."
                                   "<p>Are you sure you want to turn on "
                                   "strict country checking?"),
                               QMessageBox::Yes | QMessageBox::No,
                               QMessageBox::No);
        if (answer == QMessageBox::No)
        {
          _strictCountries->setChecked(false);
          return;
        }
      }
    }
    else if (allq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, allq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
