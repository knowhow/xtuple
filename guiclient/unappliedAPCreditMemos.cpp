/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unappliedAPCreditMemos.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "applyAPCreditMemo.h"
#include "apOpenItem.h"

unappliedAPCreditMemos::unappliedAPCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));

  _new->setEnabled(_privileges->check("MaintainAPMemos"));

  connect(_apopen, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _apopen->addColumn( tr("Doc. #"),       _itemColumn,     Qt::AlignLeft,   true,  "apopen_docnumber" );
  _apopen->addColumn( tr("Vendor"),       -1,              Qt::AlignLeft,   true,  "vendor"   );
  _apopen->addColumn( tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "apopen_amount"  );
  _apopen->addColumn( tr("Amount (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_amount"  );
  _apopen->addColumn( tr("Applied"),      _moneyColumn,    Qt::AlignRight,  true,  "apopen_paid"  );
  _apopen->addColumn( tr("Applied (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_applied"  );
  _apopen->addColumn( tr("Balance"),      _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _apopen->addColumn( tr("Currency"),     _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _apopen->addColumn( tr("Balance (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, true, "basebalance");

  if (omfgThis->singleCurrency())
    _apopen->hideColumn("currAbbr");

  if (_privileges->check("ApplyAPMemos"))
    connect(_apopen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));

  sFillList();
}

unappliedAPCreditMemos::~unappliedAPCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void unappliedAPCreditMemos::languageChange()
{
  retranslateUi(this);
}

bool unappliedAPCreditMemos::setParams(ParameterList &params)
{
  _vendorgroup->appendValue(params);
  return true;
}

void unappliedAPCreditMemos::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("UnappliedAPCreditMemos", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unappliedAPCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode",    "new");
  params.append("docType", "creditMemo");
  if (_vendorgroup->isSelectedVend())
    params.append("vend_id", _vendorgroup->vendId());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void unappliedAPCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apopen->id());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void unappliedAPCreditMemos::sFillList()
{
  MetaSQLQuery mql(
             "SELECT apopen_id, apopen_docnumber,"
             "       (vend_number || '-' || vend_name) AS vendor,"
             "       apopen_amount, (apopen_amount / apopen_curr_rate) AS base_amount, "
             "       apopen_paid, (apopen_paid / apopen_curr_rate) AS base_applied, "
             "       (apopen_amount - apopen_paid) AS balance,"
             "       (apopen_amount - apopen_paid) / apopen_curr_rate AS basebalance,"
             "	     currConcat(apopen_curr_id) AS currAbbr,"
             "       'curr' AS apopen_amount_xtnumericrole,"
             "       'curr' AS apopen_paid_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole,"
             "       'curr' AS basebalance_xtnumericrole,"
             "       'curr' AS base_amount_xtnumericrole,"
             "       'curr' AS base_applied_xtnumericrole,"
             "       0 AS basebalance_xttotalrole, "
             "       0 AS base_amount_xttotalrole, "
             "       0 AS base_applied_xttotalrole "
             "FROM apopen, vend "
             "WHERE ( (apopen_doctype='C')"
             " AND (apopen_open)"
             " AND (apopen_vend_id=vend_id)"
             "<? if exists(\"vend_id\") ?>"
             " AND (vend_id=<? value(\"vend_id\") ?>)"
             "<? elseif exists(\"vendtype_id\") ?>"
             " AND (vend_vendtype_id=<? value(\"vendtype_id\") ?>)"
             "<? elseif exists(\"vendtype_pattern\") ?>"
             " AND (vend_vendtype_id IN (SELECT vendtype_id"
             "                           FROM vendtype"
             "                           WHERE (vendtype_code ~ <? value(\"vendtype_pattern\") ?>)))"
             "<? endif ?>"
             ") "
             "ORDER BY apopen_docnumber;" );
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _apopen->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void unappliedAPCreditMemos::sApply()
{
  ParameterList params;
  params.append("apopen_id", _apopen->id());

  applyAPCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}
