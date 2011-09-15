/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspDepositsRegister.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspDepositsRegister::dspDepositsRegister(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspDepositsRegister", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Deposits Register"));
  setListLabel(tr("Transactions"));
  setReportName("DepositsRegister");
  setMetaSQLOptions("depositesRegister", "detail");

  list()->addColumn(tr("Date"),             _dateColumn,    Qt::AlignCenter, true,  "gltrans_date" );
  list()->addColumn(tr("Source"),           _orderColumn,   Qt::AlignCenter, true,  "gltrans_source" );
  list()->addColumn(tr("Doc Type"),         _orderColumn,   Qt::AlignLeft,   true,  "doctype"   );
  list()->addColumn(tr("Doc. #"),           _orderColumn,   Qt::AlignCenter, true,  "gltrans_docnumber" );
  list()->addColumn(tr("Reference"),        -1,             Qt::AlignLeft,   true,  "f_notes"   );
  list()->addColumn(tr("Account"),          _itemColumn,    Qt::AlignLeft,   true,  "f_accnt"   );
  list()->addColumn(tr("Amount Rcv'd"),     _moneyColumn,   Qt::AlignRight,  true,  "debit"  );
  list()->addColumn(tr("Credit to A/R"),    _moneyColumn,   Qt::AlignRight,  true,  "credit"  );
  list()->addColumn(tr("Balance"),          _moneyColumn,   Qt::AlignRight,  true,  "balance"  );
  list()->addColumn(tr("Currency"),      _currencyColumn,    Qt::AlignCenter, true,  "currAbbr"  );
  list()->addColumn(tr("Base Balance"),  _bigMoneyColumn,   Qt::AlignRight,  true, "base_balance"  );
}

void dspDepositsRegister::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspDepositsRegister::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    XSqlQuery qq;
    qq.prepare( "SELECT period_start, period_end "
                "FROM period "
                "WHERE (period_id=:period_id);" );
    qq.bindValue(":period_id", param.toInt());
    qq.exec();
    if (qq.first())
    {
      _dates->setStartDate(qq.value("period_start").toDate());
      _dates->setEndDate(qq.value("period_end").toDate());
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspDepositsRegister::setParams(ParameterList &params)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date(s)"),
                      tr("You must specify a valid date range.") );
    _dates->setFocus();
    return false;
  }
  else
    _dates->appendValue(params);

  params.append("invoice", tr("Invoice"));
  params.append("creditmemo", tr("Credit Memo"));

  return true;
}

