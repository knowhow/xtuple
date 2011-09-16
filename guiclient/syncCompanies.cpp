/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "syncCompanies.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QVariant>
#include <QStatusBar>

#include <dbtools.h>
#include <metasql.h>
#include <openreports.h>
#include <currcluster.h>

#include <QDebug>

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
  _company->addColumn(tr("Currency"),_itemColumn, Qt::AlignLeft, true, "company_curr");
  _company->addColumn(tr("Description"),      -1, Qt::AlignLeft,  true, "company_descrip");
  _company->addColumn(tr("Server"),  _itemColumn, Qt::AlignLeft, false, "company_server");
  _company->addColumn(tr("Port"),    _itemColumn, Qt::AlignRight,false, "company_port");
  _company->addColumn(tr("Database"),_itemColumn, Qt::AlignLeft, false, "company_database");

  _period->addColumn(tr("Name"),          -1, Qt::AlignLeft,   true, "period_name");
  _period->addColumn(tr("Start"),_dateColumn, Qt::AlignCenter, true, "period_start");
  _period->addColumn(tr("End"),  _dateColumn, Qt::AlignCenter, true, "period_end");
  _period->addColumn(tr("Closed"),  _ynColumn, Qt::AlignCenter, true, "period_closed");

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

  q.prepare("SELECT *, company_id AS company_number_xtidrole, "
            " currconcat(company_curr_id) AS company_curr, "
            " company_curr_id AS company_curr_xtidrole "
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

  QProgressDialog progress;
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setAutoClose(false);

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

    XSqlQuery co;
    co.prepare("SELECT COALESCE(company_unrlzgainloss_accnt_id,-1) AS company_unrlzgainloss_accnt_id, "
               "COALESCE(company_curr_id,-1) AS company_curr_id, "
               "COALESCE(company_yearend_accnt_id,-1) AS company_yearend_accnt_id, "
               "COALESCE(company_gainloss_accnt_id,-1) AS company_gainloss_accnt_id, "
               "COALESCE(company_dscrp_accnt_id,-1) AS company_dscrp_accnt_id "
               "FROM company WHERE (company_id=:company_id);");
    co.bindValue(":company_id", c->id());
    co.exec();
    if (co.first())
    {
      if (co.value("company_yearend_accnt_id").toInt() == -1 ||
          co.value("company_gainloss_accnt_id").toInt() == -1 ||
          co.value("company_dscrp_accnt_id").toInt() == -1)
      {
        QMessageBox::warning(this, tr("No Discrepency Account"),
                             tr("Company %1 does not appear to have "
                                "all required Accounts defined. The "
                                "data cannot safely be synchronized.")
                             .arg(c->rawValue("company_number").toString()));
        errorCount++;
        continue;
      }
      else if (co.value("company_curr_id").toInt() != CurrCluster::baseId() &&
          co.value("company_unrlzgainloss_accnt_id").toInt() == -1)
      {
        QMessageBox::warning(this, tr("No Gain/Loss Account"),
                             tr("Company %1 does not appear to have "
                                "an unrealized Gain/Loss Account defined. The "
                                "data cannot safely be synchronized.")
                             .arg(c->rawValue("company_number").toString()));
        errorCount++;
        continue;
      }
    }
    else if (co.lastError().type() != QSqlError::NoError)
    {
      systemError(this, co.lastError().databaseText(), __FILE__, __LINE__);
      errorCount++;
      continue;
    }

    QString dbURL;
    QString protocol = "psql";
    QString host = c->rawValue("company_server").toString();
    QString db   = c->rawValue("company_database").toString();
    QString port = c->rawValue("company_port").toString();
    int currid = c->id("company_curr");

    if(DEBUG)
    {
      qDebug() << "";
      qDebug() << "syncCompanies: database information-------------------";
      qDebug() << "syncCompanies: host          [ " << host << " ]";
      qDebug() << "syncCompanies: db            [ " << db << " ]";
      qDebug() << "syncCompanies: port          [ " << port << " ]";
      qDebug() << "syncCompanies: id            [ " << currid << " ]";
      qDebug() << "syncCompanies: protocol      [ " << protocol << " ]";
      qDebug() << "syncCompanies: ---------------------------------------";
      qDebug() << "";
    }

    if (progress.wasCanceled())
      break;

    buildDatabaseURL(dbURL, protocol, host, db, port);
    if (DEBUG)
      qDebug("syncCompanies::sSync() dbURL before login2 = %s", qPrintable(dbURL));

    progress.setLabelText(tr("Synchronizing Company %1 (%2)")
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

      XSqlQuery conv;
      XSqlQuery ltxn;
      ltxn.exec("BEGIN;");

      int sequence = -1;
      XSqlQuery seq;
      seq.exec("SELECT fetchGLSequence() AS sequence;");
      if (seq.first())
        sequence = seq.value("sequence").toInt();
      else if (seq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, seq.lastError().databaseText(), __FILE__, __LINE__);
        errorCount++;
        return;
      }

      // Loop and build manually to ensure proper order
      QList<XTreeWidgetItem*> period;
      for (int i = 0; i < _period->topLevelItemCount(); i++)
      {
        if (_period->topLevelItem(i)->isSelected())
        {
          bool inserted = false;
          QDate _periodStart = _period->topLevelItem(i)->rawValue("period_start").toDate();
          for (int j = 0; j < period.size(); j++)
          {
            XTreeWidgetItem *p = (XTreeWidgetItem*)(period[j]);
            QDate periodStart = p->rawValue("period_start").toDate();
            if (_periodStart < periodStart)
            {
              period.insert(j, _period->topLevelItem(i));
              inserted = true;
              break;
            }
          }
          if (!inserted)
            period.append(_period->topLevelItem(i));
        }
      }

      for (int j = 0; j < period.size(); j++)
      {
        XTreeWidgetItem *p = (XTreeWidgetItem*)(period[j]);

        if (progress.wasCanceled())
        {
          rollback.exec();
          break;
        }

        progress.setLabelText(tr("Synchronizing Company %1 (%2) \n"
                                 "Period %3")
                          .arg(c->rawValue("company_number").toString())
                          .arg(dbURL)
                          .arg(p->rawValue("period_name").toString()));

        if (p->rawValue("period_closed").toBool())
        {
          rollback.exec();
          QMessageBox::warning(this, tr("Period Closed"),
                               tr("Period %1 to %2 is closed and may not  "
                                  "be synchronized.")
                               .arg(p->rawValue("period_start").toString())
                               .arg(p->rawValue("period_end").toString())
                               );
          errorCount++;
          break;
        }

        XSqlQuery rperiod(testDB);
        rperiod.prepare("SELECT * "
                        "FROM period "
                        "WHERE ((period_start=:start)"
                        "  AND  (period_end=:end));");
        rperiod.bindValue(":start", p->rawValue("period_start"));
        rperiod.bindValue(":end",   p->rawValue("period_end"));
        rperiod.exec();
        if (rperiod.first())
        {
          if (progress.wasCanceled())
          {
            rollback.exec();
            break;
          }

          progress.setLabelText(tr("Synchronizing Company %1 (%2) \n"
                                   "Period %3: Clearing old data...")
                            .arg(c->rawValue("company_number").toString())
                            .arg(dbURL)
                            .arg(p->rawValue("period_name").toString()));
          // Clear old data
          XSqlQuery tbsync;
          tbsync.prepare("DELETE FROM trialbal "
                         "WHERE (trialbal_period_id IN ("
                         "  SELECT period_id "
                         "  FROM period "
                         "  WHERE (period_start >= :startdate))) "
                         " AND (trialbal_accnt_id IN ("
                         "  SELECT accnt_id "
                         "  FROM accnt "
                         "  WHERE ((accnt_id=trialbal_accnt_id) "
                         "   AND (accnt_company=:company_number))));"
                         "DELETE FROM gltranssync "
                         "WHERE ((gltrans_date >= :startdate)"
                         " AND (gltranssync_company_id=:company_id));");
          tbsync.bindValue(":company_id", c->id("company_number"));
          tbsync.bindValue(":startdate", p->rawValue("period_start").toDate());
          tbsync.bindValue(":company_number", c->rawValue("company_number"));
          tbsync.bindValue(":period_id", p->id());
          tbsync.exec();
          if (tbsync.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            systemError(this, rperiod.lastError().databaseText(),
                        __FILE__, __LINE__);
            errorCount++;
            break;
          }
        }
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

        if (progress.wasCanceled())
        {
          rollback.exec();
          break;
        }

        progress.setLabelText(tr("Synchronizing Company %1 (%2): \n"
                                 "Period: %3 \n"
                                 "Updating Chart of Accounts...")
                          .arg(c->rawValue("company_number").toString())
                          .arg(dbURL)
                          .arg(p->rawValue("period_name").toString()));

        int accntid = -1;
        XSqlQuery raccnt(testDB);
        raccnt.prepare("SELECT * "
                       "FROM accnt "
                       "WHERE (accnt_company=:accnt_company);");
        raccnt.bindValue(":accnt_company", c->rawValue("company_number"));
        raccnt.exec();
        progress.setMaximum(raccnt.size());
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
            accntid = laccnt.value("accnt_id").toInt();
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
            {
              laccntups.bindValue(":accnt_id",	q.value("accnt_id"));
              accntid = q.value("accnt_id").toInt();
            }
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

          // Import trans detail
          QDate prevDate = omfgThis->startOfTime();
          XSqlQuery rgl(testDB);
          XSqlQuery lgl;
          rgl.prepare("SELECT *, formatGlAccountLong(gltrans_accnt_id) AS f_accnt "
                      " FROM ( "
                      "SELECT gltrans_accnt_id, gltrans_date, gltrans_source, "
                      "  SUM(gltrans_amount) AS amount "
                      "FROM gltrans, period "
                      "WHERE ((period_id=:period_id) "
                      "  AND (gltrans_accnt_id=:accnt_id) "
                      "  AND (gltrans_amount < 0) "
                      "  AND (gltrans_posted) "
                      "  AND (NOT gltrans_deleted) "
                      "  AND (gltrans_date "
                      "       BETWEEN period_start AND period_end)) "
                      "GROUP BY gltrans_accnt_id, gltrans_source, gltrans_date "
                      "UNION ALL "
                      "SELECT gltrans_accnt_id, gltrans_date,  gltrans_source, "
                      "  SUM(gltrans_amount) AS amount "
                      "  FROM gltrans, period "
                      "WHERE ((period_id=:period_id) "
                      "  AND (gltrans_accnt_id=:accnt_id) "
                      "  AND (gltrans_amount > 0) "
                      "  AND (gltrans_posted) "
                      "  AND (NOT gltrans_deleted) "
                      "  AND (gltrans_date "
                      "       BETWEEN period_start AND period_end)) "
                      "GROUP BY gltrans_accnt_id, gltrans_source, gltrans_date) data "
                      "ORDER BY gltrans_date, gltrans_source, f_accnt; ");
          rgl.bindValue(":period_id", rperiod.value("period_id"));
          rgl.bindValue(":accnt_id",  raccnt.value("accnt_id"));
          rgl.exec();
          while (rgl.next())
          {
            // Fetch conversion rate for the date
            if (rgl.value("gltrans_date").toDate() != prevDate)
            {
              if (progress.wasCanceled())
              {
                rollback.exec();
                break;
              }

              progress.setLabelText(tr("Synchronizing Company %1 (%2) \n"
                                       "Period: %3\n"
                                       "Account: %4")
                                .arg(c->rawValue("company_number").toString())
                                .arg(dbURL)
                                .arg(p->rawValue("period_name").toString())
                                .arg(rgl.value("f_accnt").toString()));
              conv.prepare("SELECT currRate(:curr_id, :date) AS curr_rate; ");
              conv.bindValue(":curr_id", currid);
              conv.bindValue(":date", rgl.value("gltrans_date"));
              conv.exec();
              if (conv.first())
                ; // Keep going
              else if (conv.lastError().type() != QSqlError::NoError)
              {
                rollback.exec();
                QMessageBox::warning(this, tr("No Conversion Rate"),
                                     tr("The parent database for Company %1 (%2) "
                                        "does not appear to have a conversion rate "
                                        "for %3 on %4.")
                                     .arg(c->rawValue("company_number").toString())
                                     .arg(c->rawValue("company_database").toString())
                                     .arg(c->text("currency"))
                                     .arg(p->rawValue("period_end").toString())
                                     );
                errorCount++;
                break;
              }
            }

            prevDate = rgl.value("gltrans_date").toDate();

            lgl.prepare("INSERT INTO gltranssync ("
                        "  gltrans_exported, gltrans_created, "
                        "  gltrans_date, gltrans_sequence, "
                        "  gltrans_accnt_id, gltrans_source, "
                        "  gltrans_docnumber, gltrans_misc_id, "
                        "  gltrans_amount, gltrans_notes, "
                        "  gltrans_journalnumber, gltrans_posted, "
                        "  gltrans_doctype, gltrans_rec, "
                        "  gltrans_username, gltrans_deleted, "
                        "  gltranssync_company_id, "
                        "  gltranssync_period_id, gltranssync_curr_amount, "
                        "  gltranssync_curr_id, gltranssync_curr_rate) "
                        "VALUES ( "
                        "  false, now(),:date, :sequence, "
                        "  :accnt_id,:source, '', -1, "
                        "  currToBase(:curr_id, :amount, :date), "
                        "  :notes, -1, false, "
                        "  '', false, getEffectiveXtUser(), false, "
                        "  :company_id, :period_id, "
                        "  :amount, :curr_id, :curr_rate);");
            lgl.bindValue(":sequence", sequence);
            lgl.bindValue(":accnt_id", accntid);
            lgl.bindValue(":company_id", c->id("company_number"));
            lgl.bindValue(":period_id", p->id());
            lgl.bindValue(":source", rgl.value("gltrans_source"));
            lgl.bindValue(":notes", tr("Data imported from Company %1 (%2)")
                          .arg(c->rawValue("company_number").toString())
                          .arg(c->rawValue("company_database").toString()));
            lgl.bindValue(":date", rgl.value("gltrans_date"));
            lgl.bindValue(":amount", rgl.value("amount"));
            lgl.bindValue(":curr_id", currid);
            lgl.bindValue(":curr_rate", conv.value("curr_rate"));
            lgl.exec();
            if (lgl.lastError().type() != QSqlError::NoError)
            {
              rollback.exec();
              systemError(this, lgl.lastError().databaseText(),
                          __FILE__, __LINE__);
              errorCount++;
              break;
            }
          }

          if (progress.wasCanceled())
          {
            rollback.exec();
            break;
          }

          progress.setValue(progress.value()+1);
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

      if (progress.wasCanceled())
      {
        rollback.exec();
        break;
      }

      progress.setLabelText(tr("Synchronizing Company %1 (%2): Posting into trial balances...")
                            .arg(c->rawValue("company_number").toString())
                            .arg(dbURL));
      // Post into trial balance
      XSqlQuery post;
      post.prepare("SELECT postIntoTrialBalanceSync(:sequence, :notes); ");
      post.bindValue(":sequence", sequence);
      post.bindValue(":company_id", c->id("company_number"));
      post.bindValue(":notes", tr("Currency Rounding Discrepency Adjustment"));
      post.exec();
      if (post.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, post.lastError().databaseText(),
                    __FILE__, __LINE__);
        errorCount++;
        break;
      }

      XSqlQuery tbs;
      tbs.exec("SELECT trialbal_id FROM trialbalsync WHERE (trialbal_dirty); ");
      if (tbs.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, tbs.lastError().databaseText(),
                    __FILE__, __LINE__);
        errorCount++;
        break;
      }

      progress.setLabelText(tr("Synchronizing Company %1 (%2)\n"
                               "Forward updating trial balances...")
                                         .arg(c->rawValue("company_number").toString())
                                         .arg(dbURL));
      progress.setMaximum(tbs.size());
      while(tbs.next())
      {
        post.prepare("SELECT forwardUpdateTrialBalanceSync(:trialbal_id); ");
        post.bindValue(":trialbal_id", tbs.value("trialbal_id"));
        post.exec();
        if (post.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, post.lastError().databaseText(),
                      __FILE__, __LINE__);
          errorCount++;
          break;
        }

        if (progress.wasCanceled())
        {
          rollback.exec();
          break;
        }

        progress.setValue(progress.value()+1);
      }

      post.prepare("SELECT forwardUpdateTrialBalanceSync(trialbal_id) FROM trialbalsync WHERE (trialbal_dirty); ");
      post.exec();
      if (post.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, post.lastError().databaseText(),
                    __FILE__, __LINE__);
        errorCount++;
        break;
      }

      tbs.exec("SELECT trialbal_id "
               "FROM trialbalsync "
               " JOIN period ON (trialbal_period_id=period_id) "
               "WHERE ((NOT trialbalsync_curr_posted) "
               " AND (trialbalsync_curr_id != baseCurrId()))"
               "ORDER BY period_end;");
      if (tbs.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, tbs.lastError().databaseText(),
                    __FILE__, __LINE__);
        errorCount++;
        break;
      }

      progress.setLabelText(tr("Synchronizing Company %1 (%2)\n"
                               "Posting currency revaluation adjustments...")
                                         .arg(c->rawValue("company_number").toString())
                                         .arg(dbURL));
      progress.setMaximum(tbs.size());
      progress.setAutoReset(false);

      while(tbs.next())
      {
        post.prepare("SELECT postCurrAdjustSync(:trialbal_id, :adj_notes); ");
        post.bindValue(":trialbal_id", tbs.value("trialbal_id"));
        post.bindValue(":adj_notes", tr("Unrealized Gain/Loss Adjustment"));
        post.exec();
        if (post.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, post.lastError().databaseText(),
                      __FILE__, __LINE__);
          errorCount++;
          break;
        }

        if (progress.wasCanceled())
        {
          rollback.exec();
          break;
        }
        progress.setValue(progress.value()+1);
      }

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

  progress.accept();
  if (progress.wasCanceled())
    QMessageBox::critical(this, tr("Synchronizing Canceled"),
                          tr("Synchronization Canceled."));
  else
    QMessageBox::information(this, tr("Synchronizing Complete"),
                             tr("%1 Companies attempted, %2 errors encountered")
                             .arg(company.size()).arg(errorCount));
  sFillList();
}

void syncCompanies::sHandleButtons()
{
  _sync->setEnabled(_company->selectedItems().size() > 0 &&
                    _period->selectedItems().size()  > 0);
}
