/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBankrecHistory.h"

#include <QVariant>
#include "xtreewidget.h"

dspSummarizedBankrecHistory::dspSummarizedBankrecHistory(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedBankrecHistory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Bank Reconciliation History"));
  setListLabel(tr("Summarized History"));
  setReportName("SummarizedBankrecHistory");
  setMetaSQLOptions("summarizedBankrecHistory", "detail");

  list()->addColumn(tr("Posted"),             _ynColumn, Qt::AlignLeft,   true,  "bankrec_posted"   );
  list()->addColumn(tr("Post Date"),        _dateColumn, Qt::AlignCenter, true,  "bankrec_postdate" );
  list()->addColumn(tr("User"),                      -1, Qt::AlignLeft,   true,  "bankrec_username"   );
  list()->addColumn(tr("Start Date"),       _dateColumn, Qt::AlignCenter, true,  "bankrec_opendate" );
  list()->addColumn(tr("End Date"),         _dateColumn, Qt::AlignCenter, true,  "bankrec_enddate" );
  list()->addColumn(tr("Opening Bal."), _bigMoneyColumn, Qt::AlignRight,  true,  "bankrec_openbal"  );
  list()->addColumn(tr("Ending Bal."),  _bigMoneyColumn, Qt::AlignRight,  true,  "bankrec_endbal"  );
  
  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");
}

void dspSummarizedBankrecHistory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSummarizedBankrecHistory::setParams(ParameterList & params)
{
  params.append("bankaccntid", _bankaccnt->id());
  return true;
}
