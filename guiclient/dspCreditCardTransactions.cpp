/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCreditCardTransactions.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <stdlib.h>
#include <metasql.h>
#include "mqlutil.h"

#include "creditcardprocessor.h"
#include "storedProcErrorLookup.h"
#include "xtreewidget.h"

dspCreditCardTransactions::dspCreditCardTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_preauth, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(sgetCCAmount()));
  connect(_preauth, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_postPreauth, SIGNAL(clicked()), this, SLOT(sPostPreauth()));
  connect(_voidPreauth, SIGNAL(clicked()), this, SLOT(sVoidPreauth()));
  connect(_customerSelector, SIGNAL(newState(int)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newCustId(int)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newCustTypeId(int)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newTypePattern(QString)), this, SLOT(sClear()));

  _preauth->addColumn(tr("Timestamp"),   _dateColumn, Qt::AlignLeft, true,  "ccpay_transaction_datetime"  ); 
  _preauth->addColumn(tr("Cust. #"),    _orderColumn, Qt::AlignLeft,  true, "cust_number");  
  _preauth->addColumn(tr("Name"),                 -1, Qt::AlignLeft,  true, "cust_name");
  _preauth->addColumn(tr("Type"),       _orderColumn, Qt::AlignLeft,  true, "type"  );
  _preauth->addColumn(tr("Status"),  _bigMoneyColumn, Qt::AlignLeft,  true,  "status"  );
  _preauth->addColumn(tr("Document #"),           -1, Qt::AlignLeft,  true,  "docnumber"  );
  _preauth->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "ccpay_amount");
  _preauth->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");
  _preauth->addColumn(tr("Entered By"),   _userColumn,     Qt::AlignLeft,   false,  "ccpay_by_username"  );
  _preauth->addColumn(tr("Reference"),    _orderColumn,    Qt::AlignLeft,   false,  "ccpay_r_ref"  );
  _preauth->addColumn(tr("Allocated"),    _moneyColumn,    Qt::AlignRight,  false,  "allocated" );
  _preauth->addColumn(tr("Allocated Currency"), _currencyColumn, Qt::AlignLeft,   false,  "payco_currAbbr"  );
  
  if (omfgThis->singleCurrency())
    _preauth->hideColumn(2);

  if (_metrics->value("CCValidDays").toInt())
    _validDays->setValue(_metrics->value("CCValidDays").toInt());
  else
    _validDays->setValue(7);
}

dspCreditCardTransactions::~dspCreditCardTransactions()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCreditCardTransactions::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCreditCardTransactions::set( const ParameterList & pParams )
{
  XWidget::set(pParams);
  QVariant param;
  bool    valid;
  
  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _customerSelector->setCustId(param.toInt());
    sFillList();
  }

  return NoError;
}

void dspCreditCardTransactions::sFillList()
{
  _CCAmount->clear();
  
  MetaSQLQuery mql = mqlLoad("ccpayments", "list");
  ParameterList params;
  _customerSelector->appendValue(params);
  if (_processed->isChecked())
    params.append("processed");
  else if (_pending->isChecked())
    params.append("authorizations");
  if (!_expired->isChecked())
    params.append("validOnly");
  params.append("ccValidDays", _validDays->value());
  params.append("preauth",    tr("Preauthorization"));
  params.append("charge",     tr("Charge"));
  params.append("refund",     tr("Refund"));
  params.append("authorized", tr("Authorized"));
  params.append("approved",   tr("Approved"));
  params.append("declined",   tr("Declined"));
  params.append("voided",     tr("Voided"));
  params.append("noapproval", tr("No Approval Code"));
  q = mql.toQuery(params);
  _preauth->populate(q,true);
}

void dspCreditCardTransactions::sClear()
{
  _preauth->clear();
}

void dspCreditCardTransactions::sgetCCAmount()
{
  if (_preauth->altId() && _privileges->check("ProcessCreditCards"))
  {
    q.prepare("SELECT ccpay_amount, ccpay_curr_id "
               "FROM ccpay "
               " WHERE (ccpay_id = :ccpay_id);");
    q.bindValue(":ccpay_id", _preauth->id());
    if (q.exec() && q.first())
    {
      /* _CCAmount->id() defaults to customer's currency
         if CC payment is in either customer's currency or base
         set _CCAmount appropriately
         but handle it if it somehow happens to be in a 3rd currency
       */
      _postPreauth->setEnabled(true);
      _voidPreauth->setEnabled(true);
      int ccpayCurrId = q.value("ccpay_curr_id").toInt(); 
      if (ccpayCurrId == _CCAmount->baseId())
        _CCAmount->setBaseValue(q.value("ccpay_amount").toDouble());
      else if (ccpayCurrId != _CCAmount->id())
      {
        _CCAmount->setId(ccpayCurrId);
        _CCAmount->setLocalValue(q.value("ccpay_amount").toDouble());
      }
      else
        _CCAmount->setLocalValue(q.value("ccpay_amount").toDouble());
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  _CCAmount->clear();
}

void dspCreditCardTransactions::sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *pItem )
{ 
  if (((XTreeWidgetItem *)pItem)->rawValue("status") == "C")
  {
    QAction* printAct = new QAction(tr("Print Receipt"), this);
    connect(printAct, SIGNAL(triggered()), this, SLOT(sPrintCCReceipt()));
    pMenu->addAction(printAct);
  }

  if (_postPreauth->isEnabled())
  {
    QAction* postAct = new QAction(tr("Post"), this);
    connect(postAct, SIGNAL(triggered()), this, SLOT(sPostPreauth()));
    pMenu->addAction(postAct);
  }

  if (_voidPreauth->isEnabled())
  {
    QAction* voidAct = new QAction(tr("Void"), this);
    connect(voidAct, SIGNAL(triggered()), this, SLOT(sVoidPreauth()));
    pMenu->addAction(voidAct);
  }
}

void dspCreditCardTransactions::sPostPreauth()
{
  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }
  if (! cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    _CCAmount->setFocus();
    return;
  }

  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  int ccpayid   = _preauth->id();
  QString ordernum;
  int returnVal = cardproc->chargePreauthorized(-2,
						_CCAmount->localValue(),
						_CCAmount->id(),
						ordernum, ordernum, ccpayid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			 cardproc->errorMsg());
  else if (! cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
			 cardproc->errorMsg());
  else
    _CCAmount->clear();

  sFillList();
  
  _voidPreauth->setEnabled(true);
  _postPreauth->setEnabled(true);
}

void dspCreditCardTransactions::sPrintCCReceipt()
{
  CreditCardProcessor::printReceipt(_preauth->id());
}

void dspCreditCardTransactions::sVoidPreauth()
{
  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }

  if (! cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    _CCAmount->setFocus();
    return;
  }

  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  int ccpayid   = _preauth->id();
  QString ordernum;
  int returnVal = cardproc->voidPrevious(ccpayid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			 cardproc->errorMsg());
  else if (! cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
			 cardproc->errorMsg());
  else
    _CCAmount->clear();

  sFillList();

  _voidPreauth->setEnabled(true);
  _postPreauth->setEnabled(true);
}


