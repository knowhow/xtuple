/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBankrecHistory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <metasql.h>
#include <openreports.h>
#include "guiclient.h"
#include "mqlutil.h"

dspBankrecHistory::dspBankrecHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sBankaccntChanged()));
  connect(_query,   SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print,      SIGNAL(clicked()), this, SLOT(sPrint()));

  _rec->addColumn(tr("Date"),       _dateColumn, Qt::AlignCenter,true, "gltrans_date");
  _rec->addColumn(tr("Doc Number/Notes"),    -1, Qt::AlignLeft,  true, "gltrans_docnumber");
  _rec->addColumn(tr("Amount"), _bigMoneyColumn, Qt::AlignRight, true, "amount");

  _unrec->addColumn(tr("Date"),       _dateColumn, Qt::AlignCenter,true, "gltrans_date");
  _unrec->addColumn(tr("Doc Number/Notes"),    -1, Qt::AlignLeft,  true, "gltrans_docnumber");
  _unrec->addColumn(tr("Amount"), _bigMoneyColumn, Qt::AlignRight, true, "amount");
  
  _unrec->hide();
  _unrecLit->hide();
  sBankaccntChanged();
}

dspBankrecHistory::~dspBankrecHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBankrecHistory::languageChange()
{
  retranslateUi(this);
}

bool dspBankrecHistory::setParams(ParameterList &params)
{
  params.append("bankaccnt_id", _bankaccnt->id());
  params.append("bankrec_id", _bankrec->id());
  params.append("openingBalance" , tr("Opening Balance"));
  params.append("Checks" , tr("Checks"));
  params.append("Deposits" , tr("Deposits"));
  params.append("Adjustments" , tr("Adjustments"));
  params.append("closingBalance" , tr("Closing Balance"));
  params.append("reconciled" , tr("Reconciled"));
  params.append("unreconciled" , tr("Unreconciled"));
  if (_showUnreconciled->isChecked())
    params.append("showUnreconciled");
  return true;
}

void dspBankrecHistory::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("BankrecHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBankrecHistory::sBankaccntChanged()
{
  XSqlQuery bq;
  bq.prepare( "SELECT bankrec_id, (formatDate(bankrec_opendate) || '-' || formatDate(bankrec_enddate)) "
             "FROM bankrec "
             "WHERE (bankrec_bankaccnt_id=:bankaccnt_id) "
             "ORDER BY bankrec_opendate DESC, bankrec_enddate DESC; ");
  bq.bindValue(":bankaccnt_id", _bankaccnt->id());
  bq.exec();
  _bankrec->populate(bq);
}

void dspBankrecHistory::sFillList()
{
  q.prepare( "SELECT bankrec_username, "
             "       bankrec_posted,"
             "       bankrec_postdate,"
             "       bankrec_opendate,"
             "       bankrec_enddate,"
             "       bankrec_openbal,"
             "       bankrec_endbal "
             "FROM bankrec "
             "WHERE (bankrec_id=:bankrecid);" );
  q.bindValue(":bankrecid", _bankrec->id());
  q.exec();
  if(q.first())
  {
    _poster->setText(q.value("bankrec_username").toString());
    _postdate->setDate(q.value("bankrec_postdate").toDate());
    
    ParameterList params;
    params.append("treeView", true);
    if (! setParams(params))
      return;
    
    MetaSQLQuery mql = mqlLoad("bankrecHistory", "reconciled");
    q = mql.toQuery(params);
    _rec->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      _rec->expandAll();
    
    if (_showUnreconciled->isChecked())
    {
      ParameterList params2;
      params2.append("treeView", true);
      if (! setParams(params2))
        return;
      
      MetaSQLQuery mql2 = mqlLoad("bankrecHistory", "unreconciled");
      q = mql2.toQuery(params2);
      _unrec->populate(q, true);
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      else
        _unrec->expandAll();
    }
  }
}
