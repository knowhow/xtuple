/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTrialBalances.h"

#include <math.h>

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "guiclient.h"
#include "dspGLTransactions.h"
#include "storedProcErrorLookup.h"
#include "metasql.h"
#include "parameterwidget.h"
#include "xtreewidget.h"

dspTrialBalances::dspTrialBalances(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspTrialBalances", fl)
{
  setWindowTitle(tr("Trial Balances"));
  setListLabel(tr("Trial Balances"));
  setReportName("TrialBalances");
  setMetaSQLOptions("trialBalances", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  list()->addColumn(tr("Start"),       _dateColumn,     Qt::AlignCenter, true,  "period_start" );
  list()->addColumn(tr("End"),         _dateColumn,     Qt::AlignCenter, true,  "period_end" );
  list()->addColumn(tr("Account #"),   _itemColumn,     Qt::AlignLeft, true,  "account" );
  list()->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "accnt_descrip"   );
  list()->addColumn(tr("Beg. Bal."),   _bigMoneyColumn, Qt::AlignRight,  true,  "beginning"  );
  list()->addColumn("",                25,              Qt::AlignLeft,   true,  "beginningsense"   );
  list()->addColumn(tr("Debits"),      _bigMoneyColumn, Qt::AlignRight,  true,  "debits"  );
  list()->addColumn(tr("Credits"),     _bigMoneyColumn, Qt::AlignRight,  true,  "credits"  );
  list()->addColumn(tr("Difference"),  _bigMoneyColumn, Qt::AlignRight,  true,  "diff"  );
  list()->addColumn("",                25,              Qt::AlignLeft,   true,  "diffsense"   );
  list()->addColumn(tr("End Bal."),    _bigMoneyColumn, Qt::AlignRight,  true,  "ending"  );
  list()->addColumn("",                25,              Qt::AlignLeft,   true,  "endingsense"   );

  // Determine current period
  int periodid = -1;
  XSqlQuery qry;
  qry.exec("SELECT period_id "
           "FROM period "
           "WHERE (current_date BETWEEN period_start AND period_end);");
  if (qry.first())
    periodid = qry.value("period_id").toInt();

  parameterWidget()->appendComboBox(tr("Period"), "period_id", XComboBox::AccountingPeriods, QVariant(periodid));
  parameterWidget()->append(tr("GL Account"), "accnt_id",  ParameterWidget::GLAccount);
  parameterWidget()->append(tr("Show Zero Amounts"), "showZero", ParameterWidget::Exists);
  parameterWidget()->applyDefaultFilterSet();
}

void dspTrialBalances::languageChange()
{
  display::languageChange();
}

void dspTrialBalances::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;
  menuItem = pMenu->addAction(tr("View Transactions..."), this, SLOT(sViewTransactions()));

  if (_metrics->boolean("ManualForwardUpdate"))
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Forward Update"), this, SLOT(sForwardUpdate()));
  }
}

void dspTrialBalances::sViewTransactions()
{
  ParameterList params;
  params.append("accnt_id", list()->id());
  params.append("period_id", list()->altId());
  params.append("run");

  dspGLTransactions *newdlg = new dspGLTransactions();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTrialBalances::sForwardUpdate()
{
  q.prepare( "SELECT MIN(forwardUpdateTrialBalance(trialbal_id)) AS result "
             "FROM trialbal "
             "WHERE ( (trialbal_period_id=:period_id)"
             " AND (trialbal_accnt_id=:accnt_id) );");
  q.bindValue(":accnt_id", list()->id());
  q.bindValue(":period_id", list()->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("forwardUpdateTrialBalance", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspTrialBalances::sPrint()
{
  if (!_metrics->boolean("ManualForwardUpdate"))
  {
    if (!forwardUpdate())
      return;
  }
  
  display::sPrint();
}

void dspTrialBalances::sFillList()
{
  list()->clear();
  if (!_metrics->boolean("ManualForwardUpdate"))
  {
    if (!forwardUpdate())
      return;
  }
  
  display::sFillList();
}

bool dspTrialBalances::forwardUpdate()
{
  QString sql( "SELECT MIN(forwardUpdateAccount(accnt_id)) AS result "
               "FROM accnt "
               "<? if exists(\"accnt_id\") ?>"
               " WHERE (accnt_id=<? value(\"accnt_id\") ?>)"
               "<? endif ?>"
               ";" );

  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("forwardUpdateTrialBalance", result), __FILE__, __LINE__);
      return false;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  return true;
}
