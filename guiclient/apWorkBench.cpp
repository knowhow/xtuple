/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apWorkBench.h"

#include <QSqlError>
 
#include <metasql.h>
#include "openVouchers.h"
#include "selectPayments.h"
#include "selectedPayments.h"
#include "viewCheckRun.h"
#include "unappliedAPCreditMemos.h"

apWorkBench::apWorkBench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QWidget *hideme = 0;

  _vouchers = new openVouchers(this, "openVouchers", Qt::Widget);
  _vouchersTab->layout()->addWidget(_vouchers);
  hideme = _vouchers->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _vouchers->show();
  _vendorgroup->synchronize((VendorGroup*)(_vouchers->findChild<QWidget*>("_vendorgroup")));

  _payables = new selectPayments(this, "selectPayments", Qt::Widget);
  _payablesTab->layout()->addWidget(_payables);
  hideme = _payables->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _payables->show();
  _vendorgroup->synchronize((VendorGroup*)(_payables->findChild<QWidget*>("_vendorgroup")));

  _credits = new unappliedAPCreditMemos(this, "creditMemos", Qt::Widget);
  _creditsTab->layout()->addWidget(_credits);
  hideme = _credits->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _credits->show();
  _vendorgroup->synchronize((VendorGroup*)(_credits->findChild<QWidget*>("_vendorgroup")));

  _selectedPayments = new selectedPayments(this, "selectedPayments", Qt::Widget);
  _selectionsTab->layout()->addWidget(_selectedPayments);
  hideme = _selectedPayments->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _selectedPayments->show();
  _vendorgroup->synchronize((VendorGroup*)(_selectedPayments->findChild<QWidget*>("_vendorgroup")));

  QWidget * _checkRun = new viewCheckRun(this, "viewCheckRun", Qt::Widget);
  _checkRunTab->layout()->addWidget(_checkRun);
  hideme = _checkRun->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _checkRun->setWindowFlags(Qt::Widget);
  _checkRun->show();
  _vendorgroup->synchronize((VendorGroup*)(_checkRun->findChild<QWidget*>("_vendorgroup")));

  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sCalculateTotalOpen()));
  connect(_query, SIGNAL(clicked()), _vouchers, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _payables, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _credits, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _selectedPayments, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _checkRun, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculateTotalOpen()));

  sCalculateTotalOpen();
}

apWorkBench::~apWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void apWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apWorkBench::set(const ParameterList & pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool    valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendorgroup->setVendId(param.toInt());

  return NoError;
}

void apWorkBench::sCalculateTotalOpen()
{
  // copied and edited from selectPayments.cpp
  ParameterList params;
  _vendorgroup->appendValue(params);
  params.append("voucher", tr("Voucher"));
  params.append("debitMemo", tr("D/M"));

  XComboBox *bankaccnt = _payables->findChild<XComboBox*>("_bankaccnt");
  if (bankaccnt->isValid())
  {
    q.prepare( "SELECT bankaccnt_curr_id "
               "FROM bankaccnt "
               "WHERE (bankaccnt_id=:bankaccnt_id);" );
    q.bindValue(":bankaccnt_id", bankaccnt->id());
    q.exec();
    if (q.first())
      params.append("curr_id", q.value("bankaccnt_curr_id").toInt());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  
  
  if (_payables->findChild<XComboBox*>("_selectDate")->currentIndex() == 1)
    params.append("olderDate", _payables->findChild<DLineEdit*>("_onOrBeforeDate")->date());
  else if(_payables->findChild<XComboBox*>("_selectDate")->currentIndex() == 2)
  {
    params.append("startDate", _payables->findChild<DLineEdit*>("_startDate")->date());
    params.append("endDate", _payables->findChild<DLineEdit*>("_endDate")->date());
  }
  
  MetaSQLQuery due(
         "SELECT SUM((apopen_amount - apopen_paid - "
         "                      COALESCE((SELECT SUM(checkitem_amount + checkitem_discount) "
         "                                FROM checkitem, checkhead "
         "                                WHERE ((checkitem_checkhead_id=checkhead_id) "
         "                                   AND (checkitem_apopen_id=apopen_id) "
         "                                   AND (NOT checkhead_void) "
         "                                   AND (NOT checkhead_posted)) "
         "                               ), 0)) / apopen_curr_rate) AS openamount_base,"
         "       SUM(COALESCE(currToBase(apselect_curr_id, apselect_amount,"
         "                               CURRENT_DATE), 0)) AS selected_base "
         "FROM vend, apopen"
         "     LEFT OUTER JOIN apselect ON (apselect_apopen_id=apopen_id) "
         "WHERE ((apopen_open)"
         " AND (apopen_doctype IN ('V', 'D'))"
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
         "<? if exists(\"olderDate\") ?>"
         " AND (apopen_duedate < <? value(\"olderDate\") ?>)"
         "<? elseif exists(\"startDate\") ?>"
         " AND (apopen_duedate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
         "<? endif ?>"
         "<? if exists(\"curr_id\") ?>"
         " AND (apopen_curr_id=<? value(\"curr_id\") ?>)"
         "<? endif ?>"
         ");" );
  q = due.toQuery(params);
  if (q.first())
    _apopenTotal->setLocalValue(q.value("openamount_base").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // copied from unappliedAPCreditMemos and edited
  MetaSQLQuery cr(
             "SELECT SUM((apopen_amount - apopen_paid)/ apopen_curr_rate) AS basebalance "
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
             ");");
  q = cr.toQuery(params);
  if (q.first())
  {
    _apopenTotal->setLocalValue(_apopenTotal->localValue() -
                                q.value("basebalance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
