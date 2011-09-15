/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectedPayments.h"
#include "mqlutil.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "selectPayment.h"
#include "storedProcErrorLookup.h"

selectedPayments::selectedPayments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedBankAccount, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _apselect->addColumn(tr("Bank Accnt."), _itemColumn, Qt::AlignLeft  , true, "f_bank"  );
  _apselect->addColumn(tr("Vendor"),      -1,          Qt::AlignLeft  , true, "f_vendor");
  _apselect->addColumn(tr("Doc. Type"),  _orderColumn, Qt::AlignLeft  , true, "doctype");
  _apselect->addColumn(tr("Doc. #"),     _orderColumn, Qt::AlignRight , true, "apopen_docnumber");
  _apselect->addColumn(tr("Inv. #"),     _orderColumn, Qt::AlignRight , true, "apopen_invcnumber");
  _apselect->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignRight , true, "apopen_ponumber");
  _apselect->addColumn(tr("Selected"),   _moneyColumn, Qt::AlignRight , true, "apselect_amount");
  _apselect->addColumn(tr("Currency"),   _currencyColumn, Qt::AlignLeft,true, "currAbbr" );
  _apselect->addColumn(tr("Running (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, true, "apselect_running_base" );
  if (omfgThis->singleCurrency())
  {
    _apselect->hideColumn("currAbbr");
    _apselect->headerItem()->setText(8, tr("Running"));
  }

  connect(omfgThis, SIGNAL(paymentsUpdated(int, int, bool)), this, SLOT(sFillList(int)));
  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));

  sFillList();
}

selectedPayments::~selectedPayments()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectedPayments::languageChange()
{
  retranslateUi(this);
}

bool selectedPayments::setParams(ParameterList & params)
{
  _vendorgroup->appendValue(params);

  if (_selectedBankAccount->isChecked())
    params.append("bankaccntid", _bankaccnt->id());
    
  params.append("voucher",tr("Voucher"));
  params.append("debitmemo",tr("Debit Memo"));

  return true;
}

void selectedPayments::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("SelectedPaymentsList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void selectedPayments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", _apselect->id());

  selectPayment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void selectedPayments::sClear()
{
  q.prepare("SELECT clearPayment(:apselect_id) AS result;");
  q.bindValue(":apselect_id", _apselect->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("clearPayment", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPaymentsUpdated(_bankaccnt->id(), _apselect->altId(), TRUE);
}

void selectedPayments::sFillList(int pBankaccntid)
{
  if ( (pBankaccntid == -1) || (pBankaccntid == _bankaccnt->id()) )
    sFillList();
}

void selectedPayments::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("apOpenItems", "selectedpayments");
  q = mql.toQuery(params);
  _apselect->populate(q,true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
