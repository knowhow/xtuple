/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "syncCompanies.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QStatusBar>

#include <dbtools.h>
#include <metasql.h>
#include <openreports.h>

#include "login2.h"
#include "storedProcErrorLookup.h"

#define DEBUG   false

// TODO: XDialog should have a default implementation that returns FALSE
bool syncCompanies::userHasPriv(const int pMode)
{
  if (DEBUG)
    qDebug("syncCompanies::userHasPriv(%d)", pMode);
  bool retval = _privileges->check("SynchronizeCompanies");
  if (DEBUG)
    qDebug("syncCompanies::userHasPriv(%d) returning %d", pMode, retval);
  return retval;
}

// TODO: this code really belongs in XWidget
void syncCompanies::setVisible(bool visible)
{
  if (DEBUG)
    qDebug("syncCompanies::setVisible(%d) called", visible);
  if (! visible)
    XWidget::setVisible(false);

  else if (! userHasPriv())
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    deleteLater();
  }
  else
    XWidget::setVisible(true);
}

syncCompanies::syncCompanies(QWidget* parent, const char* name, Qt::WFlags fl)
: XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_company,   SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_period,    SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_sync,        SIGNAL(clicked()), this, SLOT(sSync()));

  _company->addColumn(tr("Number"),  _itemColumn, Qt::AlignCenter,true, "company_number" );
  _company->addColumn(tr("Description"),      -1, Qt::AlignLeft,  true, "company_descrip");
  _company->addColumn(tr("Server"),  _itemColumn, Qt::AlignLeft, false, "company_server");
  _company->addColumn(tr("Port"),    _itemColumn, Qt::AlignRight,false, "company_port");
  _company->addColumn(tr("Database"),_itemColumn, Qt::AlignLeft, false, "company_database");

  _period->addColumn(tr("Name"),          -1, Qt::AlignLeft,   true, "period_name");
  _period->addColumn(tr("Start"),_dateColumn, Qt::AlignCenter, true, "period_start");
  _period->addColumn(tr("End"),  _dateColumn, Qt::AlignCenter, true, "period_end");

  sFillList();
}

syncCompanies::~syncCompanies()
{
  // no need to delete child widgets, Qt does it all for us
}

void syncCompanies::languageChange()
{
  retranslateUi(this);
}

void syncCompanies::sFillList()
{
  q.exec("SELECT * "
         "FROM period "
         "ORDER BY period_start, period_end;");
  q.exec();
  _period->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT * "
            "FROM company "
            "WHERE company_external "
            "ORDER BY company_number;" );

  q.exec();
  _company->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void syncCompanies::sSync()
{
  if (DEBUG)
    qDebug("syncCompanies::sSync()");

  XSqlQuery lbaseq;
  lbaseq.exec("SELECT * FROM curr_symbol WHERE curr_base;");
  if (lbaseq.first())
    ; // keep the query results for later comparisons
  else if (lbaseq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, lbaseq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::warning(this, tr("No Base Currency"),
                         tr("<p>The parent database does not appear to have "
                            "a base currency defined. The data cannot safely "
                            "be synchronized."));
    return;
  }

  int errorCount = 0;
  QList<XTreeWidgetItem*> company = _company->selectedItems();
  for (int i = 0; i < company.size(); i++)
  {
    XTreeWidgetItem *c = (XTreeWidgetItem*)(company[i]);
    if (DEBUG)
      qDebug("syncCompanies:sSync() i %d, c %p", i, c);

    QString dbURL;
    QString protocol;
    QString host = c->rawValue("company_server").toString();
    QString db   = c->rawValue("company_database").toString();
    QString port = c->rawValue("company_port").toString();

    buildDatabaseURL(dbURL, protocol, host, db, port);
    if (DEBUG)
      qDebug("syncCompanies::sSync() dbURL before login2 = %s", qPrintable(dbURL));
    omfgThis->statusBar()->showMessage(tr("Synchronizing Company %1 (%2)")
                             .arg(c->rawValue("company_number").toString())
                             .arg(dbURL));

    ParameterList params;
    params.append("databaseURL", dbURL);
    params.append("multipleConnections");

    login2 newdlg(this, "testLogin", false);
    // disallow changing connection info
    newdlg._options->setEnabled(false);
    newdlg._demoOption->setEnabled(false);
    newdlg._otherOption->setEnabled(false);
    newdlg.set(params);
    if (newdlg.exec() == QDialog::Rejected)
    {
      QMessageBox::warning(this, tr("Skipping Database"),
                           tr("<p>Company %1 (database %2) will be "
                              "skipped at the user's request.")
                           .arg(c->rawValue("company_number").toString())
                           .arg(c->rawValue("company_database").toString()));
      errorCount++;
      continue;
    }

    dbURL = newdlg._databaseURL;
    if (DEBUG)
      qDebug("syncCompanies::sSync() dbURL after login2 = %s", qPrintable(dbURL));
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
        qDebug("syncCompanies::sSync() opened testDB!");

      XSqlQuery rmq(testDB);
      rmq.prepare("SELECT usrpriv_id "
                  "FROM usrpriv JOIN priv ON (usrpriv_priv_id=priv_id) "
                  "WHERE ((usrpriv_username=:username)"
                  "  AND  (priv_name='MaintainChartOfAccounts')) "
                  "UNION "
                  "SELECT priv_id"
                  "  FROM priv, grppriv, usrgrp"
                  " WHERE((usrgrp_grp_id=grppriv_grp_id)"
                  "   AND (grppriv_priv_id=priv_id)"
                  "   AND (usrgrp_username=:username)"
                  "   AND (priv_name='MaintainChartOfAccounts')) ;");
      rmq.bindValue(":username", newdlg.username());
      rmq.exec();
      if (rmq.first())
       ; // good - keep going
      else if (rmq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
        errorCount++;
        continue;
      }
      else
      {
        QMessageBox::warning(this, tr("No Privilege"),
                             tr("You do not have permission to view or manage "
                                "the Chart of Accounts on the child database."));
        errorCount++;
        continue;
      }

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
          errorCount++;
          continue;
        }
      }
      else if (rmq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
        continue;
      }

      rmq.exec("SELECT * FROM curr_symbol WHERE curr_base;");
      if (rmq.first())
      {
        if (rmq.value("curr_name").toString() != lbaseq.value("curr_name").toString() &&
            rmq.value("curr_symbol").toString() != lbaseq.value("curr_symbol").toString() &&
            rmq.value("curr_abbr").toString() != lbaseq.value("curr_abbr").toString())
        {
          QMessageBox::warning(this, tr("Currencies Incompatible"),
                               tr("<p>The base currencies of the child and "
                                  "parent databases do not match "
                                  "(%1, %2, %3, vs. %4, %5, %6,). The data "
                                  "cannot safely be synchronized.")
                               .arg(rmq.value("curr_name").toString())
                               .arg(rmq.value("curr_symbol").toString())
                               .arg(rmq.value("curr_abbr").toString())
                               .arg(lbaseq.value("curr_name").toString())
                               .arg(lbaseq.value("curr_symbol").toString())
                               .arg(lbaseq.value("curr_abbr").toString()));
          errorCount++;
          continue;
        }
      }
      else if (rmq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
        errorCount++;
        continue;
      }
      else
      {
        QMessageBox::warning(this, tr("No Base Currency"),
                             tr("<p>The child database does not appear to have "
                                "a base currency defined. The data cannot "
                                "safely be synchronized."));
        errorCount++;
        continue;
      }

      rmq.prepare("SELECT * FROM company WHERE (company_number=:number);");
      rmq.bindValue(":number", c->rawValue("company_number"));
      rmq.exec();
      if (rmq.first())
        ; // nothing to do
      else if (rmq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
        errorCount++;
        continue;
      }
      else
      {
        QMessageBox::warning(this, tr("No Corresponding Company"),
                             tr("<p>The child database does not appear to have "
                                "a Company %1 defined. The data cannot safely "
                                "be synchronized.")
                             .arg(c->rawValue("company_number").toString()));
        errorCount++;
        continue;
      }

      // make sure that we don't fail because of missing supporting data
      rmq.prepare("SELECT DISTINCT accnt_profit, prftcntr_descrip "
                  "FROM accnt JOIN prftcntr ON (accnt_profit=prftcntr_number) "
                  "WHERE (accnt_company=:accnt_company);");
      rmq.bindValue(":accnt_company", c->rawValue("company_number"));
      rmq.exec();
      q.prepare("SELECT * FROM prftcntr WHERE prftcntr_number=:prftcntr_number;");
      while (rmq.next())
      {
        q.bindValue(":prftcntr_number", rmq.value("accnt_profit"));
        q.exec();
        if (q.first())
          ; // nothing to do
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          errorCount++;
          // don't break/continue - do as much as we can
        }
        else
        {
          q.prepare("INSERT INTO prftcntr (prftcntr_number, prftcntr_descrip)"
                    "VALUES (:prftcntr_number, :prftcntr_descrip);");
          q.bindValue(":prftcntr_number",  rmq.value("accnt_profit"));
          q.bindValue(":prftcntr_descrip", rmq.value("prftcntr_descrip"));
          q.exec();
          if (q.lastError().type() != QSqlError::NoError)
          {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
            errorCount++;
            // don't break/continue - do as much as we can
          }
        }
      } // next profit center
      if (rmq.lastError().type() != QSqlError::NoError)
      {
	systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
        errorCount++;
	return;
      }

      rmq.prepare("SELECT DISTINCT accnt_sub "
                  "FROM accnt JOIN subaccnt ON (accnt_sub=subaccnt_number) "
                  "WHERE (accnt_company=:accnt_company);");
      rmq.bindValue(":accnt_company", c->rawValue("company_number"));
      rmq.exec();
      q.prepare("SELECT * FROM subaccnt WHERE subaccnt_number=:subaccnt_number;");
      while (rmq.next())
      {
        q.bindValue(":subaccnt_number", rmq.value("accnt_sub"));
        q.exec();
        if (q.first())
          ; // nothing to do
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          errorCount++;
          // don't break/continue - do as much as we can
        }
        else
        {
          q.prepare("INSERT INTO subaccnt (subaccnt_number, subaccnt_descrip)"
                    "VALUES (:subaccnt_number, :subaccnt_descrip);");
          q.bindValue(":subaccnt_number",  rmq.value("accnt_sub"));
          q.bindValue(":subaccnt_descrip", rmq.value("subaccnt_descrip"));
          q.exec();
          if (q.lastError().type() != QSqlError::NoError)
          {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
            errorCount++;
            // don't break/continue - do as much as we can
          }
        }
      } // next profit center
      if (rmq.lastError().type() != QSqlError::NoError)
      {
	systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
        errorCount++;
	return;
      }

      // Now for what we really want - if the periods match then upsert trialbal
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      XSqlQuery ltxn;
      ltxn.exec("BEGIN;");

      QList<XTreeWidgetItem*> period = _period->selectedItems();
      for (int j = 0; j < period.size(); j++)
      {
        XTreeWidgetItem *p = (XTreeWidgetItem*)(period[j]);
        omfgThis->statusBar()->showMessage(tr("Synchronizing Company %1 (%2): Period %3")
                                 .arg(c->rawValue("company_number").toString())
                                 .arg(dbURL)
                                 .arg(p->rawValue("period_name").toString()));

        XSqlQuery rperiod(testDB);
        rperiod.prepare("SELECT * "
                        "FROM period "
                        "WHERE ((period_start=:start)"
                        "  AND  (period_end=:end));");
        rperiod.bindValue(":start", p->rawValue("period_start"));
        rperiod.bindValue(":end",   p->rawValue("period_end"));
        rperiod.exec();
        if (rperiod.first())
          ; // keep going
        else if (rperiod.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, rperiod.lastError().databaseText(),
                      __FILE__, __LINE__);
          errorCount++;
          break;
        }
        else
        {
          rollback.exec();
          QMessageBox::warning(this, tr("No Corresponding Period"),
                               tr("<p>The child database for Company %1 (%2) "
                                  "does not appear to have an Accounting "
                                  "Period starting on %3 and ending on %4.")
                                 .arg(c->rawValue("company_number").toString())
                                 .arg(c->rawValue("company_database").toString())
                                 .arg(p->rawValue("period_start").toString())
                                 .arg(p->rawValue("period_end").toString())
                                 );
          errorCount++;
          break;
        }

        XSqlQuery raccnt(testDB);
        raccnt.prepare("SELECT * "
                       "FROM accnt "
                       "WHERE (accnt_company=:accnt_company);");
        raccnt.bindValue(":accnt_company", c->rawValue("company_number"));
        raccnt.exec();
        while (raccnt.next())
        {
          XSqlQuery laccnt;
          laccnt.prepare("SELECT * "
                         "FROM accnt "
                         "WHERE ((accnt_company=:accnt_company)"
                         "  AND  (accnt_profit=:accnt_profit)"
                         "  AND  (accnt_number=:accnt_number)"
                         "  AND  (accnt_sub=:accnt_sub));");
          laccnt.bindValue(":accnt_company",raccnt.value("accnt_company"));
          laccnt.bindValue(":accnt_profit", raccnt.value("accnt_profit"));
          laccnt.bindValue(":accnt_number", raccnt.value("accnt_number"));
          laccnt.bindValue(":accnt_sub",    raccnt.value("accnt_sub"));
          laccnt.exec();

          XSqlQuery laccntups;  // update/insert local account table
          if (laccnt.first())
          {
            laccntups.prepare("UPDATE accnt SET "
                              "    accnt_descrip=:accnt_descrip,"
                              "    accnt_comments=:accnt_comments,"
                              "    accnt_type=:accnt_type,"
                              "    accnt_extref=:accnt_extref,"
                              "    accnt_closedpost=:accnt_closedpost,"
                              "    accnt_forwardupdate=:accnt_forwardupdate,"
                              "    accnt_subaccnttype_code=:accnt_subaccnttype_code,"
                              "    accnt_curr_id=:accnt_curr_id "
                              "WHERE (accnt_id=:accnt_id);");
            laccntups.bindValue(":accnt_id",	laccnt.value("accnt_id"));
          }
          else if (laccnt.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, laccnt.lastError().databaseText(),
                        __FILE__, __LINE__);
            errorCount++;
            break;
          }
          else
          {
            laccntups.prepare("INSERT INTO accnt ("
                              "    accnt_id, accnt_number, accnt_descrip,"
                              "    accnt_comments, accnt_profit, accnt_sub,"
                              "    accnt_type, accnt_extref, accnt_company, "
                              "    accnt_closedpost, accnt_forwardupdate, "
                              "    accnt_subaccnttype_code, accnt_curr_id) "
                              "VALUES ("
                              "    :accnt_id, :accnt_number,:accnt_descrip,"
                              "    :accnt_comments,:accnt_profit,:accnt_sub,"
                              "    :accnt_type,:accnt_extref,:accnt_company, "
                              "    :accnt_closedpost,:accnt_forwardupdate, "
                              "    :accnt_subaccnttype_code,:accnt_curr_id);");
            q.prepare("SELECT NEXTVAL('accnt_accnt_id_seq') AS accnt_id;");
            q.exec();
            if (q.first())
              laccntups.bindValue(":accnt_id",	q.value("accnt_id"));
            else if (q.lastError().type() != QSqlError::NoError)
            {
              systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
              errorCount++;
              break;
            }
          }

          laccntups.bindValue(":accnt_number",	  raccnt.value("accnt_number"));
          laccntups.bindValue(":accnt_descrip",	  raccnt.value("accnt_descrip"));
          laccntups.bindValue(":accnt_comments",  raccnt.value("accnt_comments"));
          laccntups.bindValue(":accnt_profit",	  raccnt.value("accnt_profit"));
          laccntups.bindValue(":accnt_sub",	  raccnt.value("accnt_sub"));
          laccntups.bindValue(":accnt_type",	  raccnt.value("accnt_type"));
          laccntups.bindValue(":accnt_extref",	  raccnt.value("accnt_extref"));
          laccntups.bindValue(":accnt_company",	  raccnt.value("accnt_company"));
          laccntups.bindValue(":accnt_closedpost",raccnt.value("accnt_closedpost"));
          laccntups.bindValue(":accnt_forwardupdate",raccnt.value("accnt_forwardupdate"));
          laccntups.bindValue(":accnt_subaccnttype_code",raccnt.value("accnt_subaccnttype_code"));
          laccntups.bindValue(":accnt_curr_id",	  raccnt.value("accnt_curr_id"));

          laccntups.exec();
          if (laccntups.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, laccntups.lastError().databaseText(),
                        __FILE__, __LINE__);
            errorCount++;
            break;
          }

          // select trial balances from remote using remote ids
          XSqlQuery rtb(testDB);
          rtb.prepare("SELECT * "
                      "FROM trialbal "
                      "WHERE ((trialbal_period_id=:period_id)"
                      "  AND  (trialbal_accnt_id=:accnt_id));");
          rtb.bindValue(":period_id", rperiod.value("period_id"));
          rtb.bindValue(":accnt_id",  raccnt.value("accnt_id"));
          rtb.exec();
          if (rtb.first())
            ; // keep going
          else if (rtb.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, rtb.lastError().databaseText(),
                        __FILE__, __LINE__);
            errorCount++;
            break;
          }
          else
          {
            // TODO: warn the user?
            continue;
          }

          // update local trial balances using local ids
          XSqlQuery ltb;
          ltb.prepare("SELECT * "
                      "FROM trialbal "
                      "WHERE ((trialbal_period_id=:period_id)"
                      "  AND  (trialbal_accnt_id=:accnt_id));");
          ltb.bindValue(":period_id",p->id());
          ltb.bindValue(":accnt_id", laccnt.value("accnt_id"));
          ltb.exec();

          XSqlQuery ltbups;
          if (ltb.first())
          {
            ltbups.prepare("UPDATE trialbal SET "
                           "    trialbal_period_id=:trialbal_period_id,"
                           "    trialbal_accnt_id=:trialbal_accnt_id,"
                           "    trialbal_beginning=:trialbal_beginning,"
                           "    trialbal_ending=:trialbal_ending,"
                           "    trialbal_credits=:trialbal_credits,"
                           "    trialbal_debits=:trialbal_debits,"
                           "    trialbal_dirty=:trialbal_dirty,"
                           "    trialbal_yearend=:trialbal_yearend "
                           "WHERE (trialbal_id=:trialbal_id);");
            ltbups.bindValue(":trialbal_id",	 ltb.value("trialbal_id"));
          }
          else if (ltb.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, ltb.lastError().databaseText(),
                        __FILE__, __LINE__);
            errorCount++;
            break;
          }
          else
          {
            ltbups.prepare("INSERT INTO trialbal (trialbal_id, "
                           "    trialbal_period_id, trialbal_accnt_id,"
                           "    trialbal_beginning, trialbal_ending,"
                           "    trialbal_credits,   trialbal_debits,"
                           "    trialbal_dirty,     trialbal_yearend) "
                           "VALUES (:trialbal_id, "
                           "   :trialbal_period_id,:trialbal_accnt_id,"
                           "   :trialbal_beginning,:trialbal_ending,"
                           "   :trialbal_credits,  :trialbal_debits, "
                           "   :trialbal_dirty,    :trialbal_yearend);");
            q.prepare("SELECT NEXTVAL('trialbal_trialbal_id_seq') AS trialbal_id;");
            q.exec();
            if (q.first())
              ltbups.bindValue(":trialbal_id",	q.value("trialbal_id"));
            else if (q.lastError().type() != QSqlError::NoError)
            {
              systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
              errorCount++;
              break;
            }
          }

          // ids have to match the local db, not the remote
          ltbups.bindValue(":trialbal_period_id",p->id());
          ltbups.bindValue(":trialbal_accnt_id", laccnt.value("accnt_id"));
          ltbups.bindValue(":trialbal_beginning",rtb.value("trialbal_beginning"));
          ltbups.bindValue(":trialbal_ending",	 rtb.value("trialbal_ending"));
          ltbups.bindValue(":trialbal_credits",	 rtb.value("trialbal_credits"));
          ltbups.bindValue(":trialbal_debits",	 rtb.value("trialbal_debits"));
          ltbups.bindValue(":trialbal_dirty",	 rtb.value("trialbal_dirty"));
          ltbups.bindValue(":trialbal_yearend",	 rtb.value("trialbal_yearend"));

          ltbups.exec();
          if (ltbups.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, ltbups.lastError().databaseText(),
                        __FILE__, __LINE__);
            errorCount++;
            break;
          }
        } // for each remote g/l account
        if (raccnt.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, raccnt.lastError().databaseText(),
                      __FILE__, __LINE__);
          errorCount++;
          break;
        }
      } // for each selected period

      ltxn.exec("COMMIT;");
    }
    else
    {
      QMessageBox::warning(this, tr("Could Not Connect"),
                           tr("<p>Could not connect to the child database "
                              "with these connection parameters."));
      errorCount++;
      continue;
    }
  } // for each selected company

  omfgThis->statusBar()->showMessage(tr("Synchronizing Complete: "
                              "%1 Companies attempted, %2 errors encountered")
                           .arg(company.size()).arg(errorCount));
  sFillList();
}

void syncCompanies::sHandleButtons()
{
  _sync->setEnabled(_company->selectedItems().size() > 0 &&
                    _period->selectedItems().size()  > 0);
}
