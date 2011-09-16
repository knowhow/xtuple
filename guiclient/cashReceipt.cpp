/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cashReceipt.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <stdlib.h>

#include "cashReceiptItem.h"
#include "cashReceiptMiscDistrib.h"
#include "creditCard.h"
#include "creditcardprocessor.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

#define TR(a)	cashReceipt::tr(a)

const struct {
  const char * full;
  QString abbr;
  bool    cc;
} _fundsTypes[] = {
  { QT_TRANSLATE_NOOP("cashReceipt", "Check"),            "C", false },
  { QT_TRANSLATE_NOOP("cashReceipt", "Certified Check"),  "T", false },
  { QT_TRANSLATE_NOOP("cashReceipt", "Master Card"),      "M", true  },
  { QT_TRANSLATE_NOOP("cashReceipt", "Visa"),             "V", true  },
  { QT_TRANSLATE_NOOP("cashReceipt", "American Express"), "A", true  },
  { QT_TRANSLATE_NOOP("cashReceipt", "Discover Card"),    "D", true  },
  { QT_TRANSLATE_NOOP("cashReceipt", "Other Credit Card"),"R", true  },
  { QT_TRANSLATE_NOOP("cashReceipt", "Cash"),             "K", false },
  { QT_TRANSLATE_NOOP("cashReceipt", "Wire Transfer"),    "W", false },
  { QT_TRANSLATE_NOOP("cashReceipt", "Other"),            "O", false }
};

cashReceipt::cashReceipt(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_received, SIGNAL(lostFocus()), this, SLOT(sUpdateBalance()));
  connect(_applyToBalance, SIGNAL(clicked()), this, SLOT(sApplyToBalance()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_applyLineBalance, SIGNAL(clicked()), this, SLOT(sApplyLineBalance()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_searchDocNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged()));
  connect(_applied, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_miscDistribs, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_received, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sFillApplyList()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sFillMiscList()));
  connect(_received, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillApplyList()));
  connect(_received, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillMiscList()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sChangeCurrency(int)));
  connect(_distDate, SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
  connect(_applDate, SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
  connect(_credits, SIGNAL(toggled(bool)), this, SLOT(sFillApplyList()));
  if (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _tab->removeTab(_tab->indexOf(_creditCardTab));
  }
  else
  {
    connect(_newCC, SIGNAL(clicked()), this, SLOT(sNewCreditCard()));
    connect(_editCC, SIGNAL(clicked()), this, SLOT(sEditCreditCard()));
    connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
    connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
    connect(_downCC, SIGNAL(clicked()), this, SLOT(sMoveDown()));
    connect(_fundsType, SIGNAL(activated(int)), this, SLOT(setCreditCard()));
  }

  QButtonGroup * bg = new QButtonGroup(this);
  bg->addButton(_balCreditMemo);
  bg->addButton(_balCustomerDeposit);

  _applied->clear();

  _CCCVV->setValidator(new QIntValidator(100, 9999, this));

  _bankaccnt->setType(XComboBox::ARBankAccounts);
  _salescat->setType(XComboBox::SalesCategoriesActive);

  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter, true, "doctype");
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignCenter, true, "aropen_docnumber");
  _aropen->addColumn(tr("Ord. #"),    _orderColumn,    Qt::AlignCenter, true, "aropen_ordernumber");
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter, true, "aropen_docdate");
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter, true, "aropen_duedate");
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight,  true, "balance");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "balance_curr");
  _aropen->addColumn(tr("Applied"),   _bigMoneyColumn, Qt::AlignRight,  true, "applied");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "applied_curr");
  _aropen->addColumn(tr("Discount"),  _moneyColumn,    Qt::AlignRight , true, "discount" );
  _aropen->addColumn(tr("All Pending"),_moneyColumn,   Qt::AlignRight,  true, "pending");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "pending_curr");

  _cashrcptmisc->addColumn(tr("Account #"), _itemColumn,     Qt::AlignCenter, true, "account");
  _cashrcptmisc->addColumn(tr("Notes"),     -1,              Qt::AlignLeft,  true, "firstline");
  _cashrcptmisc->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight, true, "cashrcptmisc_amount");

  _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
  _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
  _cc->addColumn(tr("Number"),  _itemColumn, Qt::AlignRight,true, "f_number");
  _cc->addColumn(tr("Active"),  _itemColumn, Qt::AlignLeft, true, "ccard_active");
  _cc->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "ccard_name");
  _cc->addColumn(tr("Expiration Date"),  -1, Qt::AlignLeft, true, "expiration");

  for (unsigned int i = 0; i < sizeof(_fundsTypes) / sizeof(_fundsTypes[1]); i++)
  {
    // only show credit card funds types if the user can process cc transactions
    if (! _fundsTypes[i].cc ||
        (_fundsTypes[i].cc && _metrics->boolean("CCAccept") &&
         _privileges->check("ProcessCreditCards")) )
      _fundsType->append(i, tr(_fundsTypes[i].full), _fundsTypes[i].abbr);
  }

  if (!_metrics->boolean("CCAccept") && ! _privileges->check("ProcessCreditCards"))
    _tab->removeTab(_tab->indexOf(_creditCardTab));

  if(_metrics->boolean("HideApplyToBalance"))
    _applyToBalance->hide();

  if(_metrics->boolean("EnableCustomerDeposits"))
    _balCustomerDeposit->setChecked(true);
  else
  {
   _applyBalLit->hide();
   _balCreditMemo->hide();
   _balCustomerDeposit->hide();
  }

  _overapplied = false;
  _cashrcptid = -1;
  _posted = false;
}

cashReceipt::~cashReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceipt::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceipt::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
  {
    _cashrcptid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _transType = cNew;

      q.exec("SELECT fetchCashRcptNumber() AS number;");
      if (q.first())
        _number->setText(q.value("number").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      q.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
      if (q.first())
        _cashrcptid = q.value("cashrcpt_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _distDate->setDate(omfgThis->dbDate(), true);
      _applDate->setDate(omfgThis->dbDate(), true);
      _cust->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
	  _transType = cEdit;
      //_tab->removeTab(_tab->indexOf(_creditCardTab));

      _cust->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
	  _transType = cView;
      _tab->removeTab(_tab->indexOf(_creditCardTab));

      _cust->setReadOnly(TRUE);
      _received->setEnabled(FALSE);
      _fundsType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _docDate->setEnabled(FALSE);
      _bankaccnt->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
      _applDate->setEnabled(false);
      _aropen->setEnabled(FALSE);
      _cashrcptmisc->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _applyToBalance->setEnabled(FALSE);
      _add->setEnabled(FALSE);
      _balCreditMemo->setEnabled(false);
      _balCustomerDeposit->setEnabled(false);
      _save->hide();
      _close->setText(tr("&Close"));
      _altAccnt->setEnabled(false);
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _close->setFocus();
    }

    // if this cash receipt was by credit card cash then prevent changes
    _ccEdit = (_mode == cEdit) &&
	      (_origFunds == "A" || _origFunds == "D" ||
	       _origFunds == "M" || _origFunds == "V");

    if(_ccEdit)
    {
      _received->setEnabled(FALSE);
      _fundsType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _docDate->setEnabled(FALSE);
      _bankaccnt->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
      _applDate->setEnabled(FALSE);
    }

  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());
    
  param = pParams.value("docnumber", & valid);
  if (valid)
    _searchDocNum->setText(param.toString());

  return NoError;
}

void cashReceipt::sApplyToBalance()
{
  if(!save(true))
    return;

  XSqlQuery applyToBal;
  applyToBal.prepare( "UPDATE cashrcpt "
             "SET cashrcpt_cust_id=:cust_id, "
             "    cashrcpt_curr_id=:curr_id, "
             "    cashrcpt_docdate=:docdate "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  applyToBal.bindValue(":cust_id", _cust->id());
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.bindValue(":docdate", _docDate->date());
  applyToBal.exec();

  applyToBal.prepare("SELECT applyCashReceiptToBalance(:cashrcpt_id, "
                     "             :amount, :curr_id, :inclCredits) AS result;");
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":amount", _received->localValue());
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.bindValue(":inclCredits", _credits->isChecked());
  applyToBal.exec();
  if (applyToBal.lastError().type() != QSqlError::NoError)
      systemError(this, applyToBal.lastError().databaseText(), __FILE__, __LINE__);

  sFillApplyList();
}

void cashReceipt::sApply()
{
  if(_mode == cNew)
  {
    if(!save(TRUE))
	{
      return;
	}
  }
	  
  bool update  = FALSE;
  QList<XTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    ParameterList params;

    if(cursor->altId() != -1)
    {
      params.append("mode", "edit");
      params.append("cashrcptitem_id", cursor->altId());
      params.append("amount_to_apply", _received->localValue());
    }
    else
    {
      params.append("mode", "new");
      params.append("amount_to_apply", _balance->localValue());
    }
    params.append("cashrcpt_id", _cashrcptid);
    params.append("aropen_id", cursor->id());
    params.append("curr_id", _received->id());
	
    cashReceiptItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      update = TRUE;
  }

  if (update)
    sFillApplyList();
}

void cashReceipt::sApplyLineBalance()
{
  if(!save(true))
    return;

  XSqlQuery applyToBal;
  applyToBal.prepare( "UPDATE cashrcpt "
                      "SET cashrcpt_cust_id=:cust_id, "
                      "    cashrcpt_curr_id=:curr_id, "
                      "    cashrcpt_docdate=:docdate "
                      "WHERE (cashrcpt_id=:cashrcpt_id);" );
  applyToBal.bindValue(":cust_id", _cust->id());
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.bindValue(":docdate", _docDate->date());
  applyToBal.exec();
	  
  QList<XTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.prepare( "SELECT applycashreceiptlinebalance(:cashrcpt_id,"
	       "            :cashrcptitem_aropen_id,:amount,:curr_id) AS result;" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.bindValue(":cashrcptitem_aropen_id", cursor->id());
    if (_aropen->rawValue("doctype").toString() == "I" ||
        _aropen->rawValue("doctype").toString() == "D")
      q.bindValue(":amount", _received->localValue());
    else
    {
      // Total credit available is unapplied credit in receipt currency
      double credit = _received->convert(_aropen->rawValue("balance_curr").toInt(),
                                         _received->id(),
                                         _aropen->rawValue("balance").toDouble(),
                                         _distDate->date());
      q.bindValue(":amount", credit);
    }
    q.bindValue(":curr_id", _received->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	    systemError(this, storedProcErrorLookup("applyCashReceiptLineBalance", result), __FILE__, __LINE__);
	    return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillApplyList();
}

void cashReceipt::sClear()
{
  QList<XTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.prepare( "DELETE FROM cashrcptitem "
               " WHERE ((cashrcptitem_aropen_id=:aropen_id) "
               " AND (cashrcptitem_cashrcpt_id=:cashrcpt_id));" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.bindValue(":aropen_id", cursor->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillApplyList();
  sUpdateBalance();
}

void cashReceipt::sAdd()
{
  if(_mode == cNew)
    if(!save(true))
      return;
	  
  ParameterList params;
  params.append("mode", "new");
  params.append("cashrcpt_id", _cashrcptid);
  params.append("curr_id", _received->id());
  params.append("effective", _received->effective());

  cashReceiptMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcptmisc_id", _cashrcptmisc->id());
  params.append("curr_id", _received->id());
  params.append("effective", _received->effective());

  cashReceiptMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sDelete()
{
  q.prepare( "DELETE FROM cashrcptmisc "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  q.bindValue(":cashrcptmisc_id", _cashrcptmisc->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillMiscList();
}

void cashReceipt::close()
{
  if (_transType == cNew && _cashrcptid >= 0)
  {
    q.prepare("SELECT deleteCashRcpt(:cashrcpt_id) AS result;");
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteCashRcpt", result));
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare("SELECT releaseCashRcptNumber(:number);");
    q.bindValue(":number", _number->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  XWidget::close();
}

void cashReceipt::sSave()
{
  if (save(false))
  {
    omfgThis->sCashReceiptsUpdated(_cashrcptid, TRUE);
    _cashrcptid = -1;

    close();
  }
}

bool cashReceipt::save(bool partial)
{
  if (_overapplied &&
      QMessageBox::question(this, tr("Overapplied?"),
                            tr("This Cash Receipt appears to apply too much to"
                               " at least one of the Open Items. Do you want "
                               "to save the current applications anyway?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return FALSE;

  int _bankaccnt_curr_id = -1;
  QString _bankaccnt_currAbbr;
  q.prepare( "SELECT bankaccnt_curr_id, "
             "       currConcat(bankaccnt_curr_id) AS currAbbr "
             "  FROM bankaccnt "
             " WHERE (bankaccnt_id=:bankaccnt_id);");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    _bankaccnt_curr_id = q.value("bankaccnt_curr_id").toInt();
    _bankaccnt_currAbbr = q.value("currAbbr").toString();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return FALSE;
  }

  if (_received->currencyEnabled() && _received->id() != _bankaccnt_curr_id &&
      QMessageBox::question(this, tr("Cash Receipt Transaction Not In Bank Currency"),
                          tr("<p>This transaction is specified in %1 while the "
                             "Bank Account is specified in %2. Do you wish to "
                             "convert at the current Exchange Rate?"
			     "<p>If not, click NO "
                             "and change the Bank Account in the POST TO field.")
                          .arg(_received->currAbbr())
                          .arg(_bankaccnt_currAbbr),
                          QMessageBox::Yes|QMessageBox::Escape,
                          QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
    _received->setFocus();
    return FALSE;
  }
  _received->setCurrencyDisabled(true);

  if (!partial)
  {
    if (!_ccEdit &&
        (_fundsType->code() == "A" || _fundsType->code() == "D" ||
         _fundsType->code() == "M" || _fundsType->code() == "V"))
    {
      if (_cc->id() <= -1)
      {
        QMessageBox::warning(this, tr("Select a Credit Card"),
                             tr("Please select a Credit Card from the list "
                                "before continuing."));
        _tab->setCurrentIndex(_tab->indexOf(_creditCardTab));
        _cc->setFocus();
        return FALSE;
      }
      CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
      if (! cardproc)
      {
        QMessageBox::critical(this, tr("Credit Card Processing Error"),
                              CreditCardProcessor::errorMsg());
        return FALSE;
      }

      _save->setEnabled(false);
      int ccpayid = -1;
      QString ordernum = _docNumber->text().isEmpty() ?
	  	      QString::number(_cashrcptid) : _docNumber->text();
      int returnVal = cardproc->charge(_cc->id(),
				     _CCCVV->text().isEmpty() ? -1 : _CCCVV->text().toInt(),
				     _received->localValue(),
				     0, false, 0, 0,
				     _received->id(),
				     ordernum, ordernum, ccpayid,
				     QString("cashrcpt"), _cashrcptid);
      if (returnVal < 0)
        QMessageBox::critical(this, tr("Credit Card Processing Error"),
		  	    cardproc->errorMsg());
      else if (returnVal > 0)
        QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			   cardproc->errorMsg());
      else if (! cardproc->errorMsg().isEmpty())
        QMessageBox::information(this, tr("Credit Card Processing Note"),
			   cardproc->errorMsg());

      _save->setEnabled(true);
      if (returnVal < 0)
        return FALSE;
    }
  }

  if (!_distDate->isValid())
    _distDate->setDate(omfgThis->dbDate());

  if (_mode == cNew)
    q.prepare( "INSERT INTO cashrcpt "
               "( cashrcpt_id, cashrcpt_cust_id, cashrcpt_distdate, cashrcpt_amount,"
               "  cashrcpt_fundstype, cashrcpt_bankaccnt_id, cashrcpt_curr_id, "
               "  cashrcpt_usecustdeposit, cashrcpt_docnumber, cashrcpt_docdate, "
               "  cashrcpt_notes, cashrcpt_salescat_id, cashrcpt_number, cashrcpt_applydate, cashrcpt_discount ) "
               "VALUES "
               "( :cashrcpt_id, :cashrcpt_cust_id, :cashrcpt_distdate, :cashrcpt_amount,"
               "  :cashrcpt_fundstype, :cashrcpt_bankaccnt_id, :curr_id, "
               "  :cashrcpt_usecustdeposit, :cashrcpt_docnumber, :cashrcpt_docdate, "
               "  :cashrcpt_notes, :cashrcpt_salescat_id, :cashrcpt_number, :cashrcpt_applydate, :cashrcpt_discount );" );
  else
    q.prepare( "UPDATE cashrcpt "
	       "SET cashrcpt_cust_id=:cashrcpt_cust_id,"
	       "    cashrcpt_amount=:cashrcpt_amount,"
	       "    cashrcpt_fundstype=:cashrcpt_fundstype,"
	       "    cashrcpt_docnumber=:cashrcpt_docnumber,"
	       "    cashrcpt_docdate=:cashrcpt_docdate,"
	       "    cashrcpt_bankaccnt_id=:cashrcpt_bankaccnt_id,"
	       "    cashrcpt_distdate=:cashrcpt_distdate,"
	       "    cashrcpt_notes=:cashrcpt_notes, "
	       "    cashrcpt_salescat_id=:cashrcpt_salescat_id, "
	       "    cashrcpt_curr_id=:curr_id,"
	       "    cashrcpt_usecustdeposit=:cashrcpt_usecustdeposit,"
           "    cashrcpt_applydate=:cashrcpt_applydate,"
		   "    cashrcpt_discount=:cashrcpt_discount "
		   "WHERE (cashrcpt_id=:cashrcpt_id);" );

  q.bindValue(":cashrcpt_id", _cashrcptid);
  q.bindValue(":cashrcpt_number", _number->text());
  q.bindValue(":cashrcpt_cust_id", _cust->id());
  q.bindValue(":cashrcpt_amount", _received->localValue());
  q.bindValue(":cashrcpt_fundstype", _fundsType->code());
  q.bindValue(":cashrcpt_docnumber", _docNumber->text());
  q.bindValue(":cashrcpt_docdate", _docDate->date());
  q.bindValue(":cashrcpt_bankaccnt_id", _bankaccnt->id());
  q.bindValue(":cashrcpt_distdate", _distDate->date());
  q.bindValue(":cashrcpt_applydate", _applDate->date());
  q.bindValue(":cashrcpt_notes",          _notes->toPlainText().trimmed());
  q.bindValue(":cashrcpt_usecustdeposit", QVariant(_balCustomerDeposit->isChecked()));
  q.bindValue(":cashrcpt_discount", _discount->localValue());
  q.bindValue(":curr_id", _received->id());
  if(_altAccnt->isChecked())
    q.bindValue(":cashrcpt_salescat_id", _salescat->id());
  else
    q.bindValue(":cashrcpt_salescat_id", -1);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return FALSE;
  }
  _mode=cEdit;
  return TRUE;
}

void cashReceipt::sPopulateCustomerInfo(int)
{
  if (_mode == cNew)
  {
    XSqlQuery cust;
    cust.prepare("SELECT cust_curr_id FROM cust WHERE cust_id = :cust_id;");
    cust.bindValue(":cust_id", _cust->id());
    cust.exec();
    if (cust.first())
      _received->setId(cust.value("cust_curr_id").toInt());
    else if (cust.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillApplyList();
}

void cashReceipt::sFillApplyList()
{
  if (_cust->isValid())
  {
    _cust->setReadOnly(TRUE);

    _aropen->clear();
    MetaSQLQuery mql = mqlLoad("arOpenApplications", "detail");
    ParameterList params;
    params.append("cashrcpt_id", _cashrcptid);
    params.append("cust_id",     _cust->id());
    params.append("debitMemo",   tr("Debit Memo"));
    params.append("invoice",     tr("Invoice"));
    params.append("creditMemo",  tr("Credit Memo"));
    params.append("cashdeposit", tr("Customer Deposit"));
    if (_posted)
      params.append("posted", true);
    else if (!_credits->isChecked())
      params.append("noCredits");
    XSqlQuery apply;
    apply = mql.toQuery(params);
    _aropen->populate(apply, true);
    if (apply.lastError().type() != QSqlError::NoError)
    {
      systemError(this, apply.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    _overapplied = false;
    XTreeWidgetItem *last;
    for (int i = 0; i < _aropen->topLevelItemCount(); i++)
    {
      last = _aropen->topLevelItem(i);
      if (last->data(9, Qt::UserRole).toMap().value("raw").toDouble() >
          last->data(5, Qt::UserRole).toMap().value("raw").toDouble())
      {
        _overapplied = true;
        break;
      }
    }

    apply.prepare( "SELECT SUM(COALESCE(cashrcptitem_amount, 0)) AS total "
               "FROM cashrcptitem "
               "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id);" );
    apply.bindValue(":cashrcpt_id", _cashrcptid);
    apply.exec();
    if (apply.first())
      _applied->setLocalValue(apply.value("total").toDouble());
    else if (apply.lastError().type() != QSqlError::NoError)
    {
      systemError(this, apply.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    apply.prepare("select max(aropen_docdate) AS mindate FROM aropen, cashrcptitem WHERE cashrcptitem_aropen_id=aropen_id AND cashrcptitem_cashrcpt_id=:id;");
    apply.bindValue(":id", _cashrcptid);
    apply.exec();
    if(apply.first())
    {
      _mindate = apply.value("mindate").toDate();
      if(_mindate > _applDate->date())
        _applDate->setDate(_mindate);
    }
    
	XSqlQuery discount ;

	discount.prepare( "SELECT SUM(COALESCE(cashrcptitem_discount, 0.00)) AS disc "
                      "FROM cashrcptitem "
                      "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id);" );
    discount.bindValue(":cashrcpt_id", _cashrcptid);
	discount.exec();
    if (discount.first())
	{
      _discount->setLocalValue(discount.value("disc").toDouble());
	  sUpdateBalance();
	}
    else if (discount.lastError().type() != QSqlError::NoError)
    {
      systemError(this, discount.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

  }
  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());

}

void cashReceipt::sFillMiscList()
{
  XSqlQuery misc;
  misc.prepare("SELECT cashrcptmisc_id,"
               "       formatGLAccount(cashrcptmisc_accnt_id) AS account,"
               "       firstLine(cashrcptmisc_notes) AS firstline, "
               "       cashrcptmisc_amount,"
               "       'curr' AS cashrcptmisc_amount_xtnumericrole "
               "FROM cashrcptmisc "
               "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  _cashrcptmisc->populate(misc);
  if (misc.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  misc.prepare( "SELECT SUM(cashrcptmisc_amount) AS total "
             "FROM cashrcptmisc "
             "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  if (misc.first())
    _miscDistribs->setLocalValue(misc.value("total").toDouble());
  else if (misc.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _miscDistribs->clear();
  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());
}

void cashReceipt::sUpdateBalance()
{
  _balance->setLocalValue(_received->localValue() - _applied->localValue() -
                          _miscDistribs->localValue());
  if (!_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("red"));
  else
    _balance->setPaletteForegroundColor(QColor("black"));
}

void cashReceipt::populate()
{
  q.prepare( "SELECT *,"
             "       COALESCE(cashrcpt_applydate, cashrcpt_distdate) AS applydate "
             "FROM cashrcpt "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  q.bindValue(":cashrcpt_id", _cashrcptid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("cashrcpt_cust_id").toInt());
    _number->setText(q.value("cashrcpt_number").toString());
    _received->set(q.value("cashrcpt_amount").toDouble(),
                   q.value("cashrcpt_curr_id").toInt(),
                   q.value("cashrcpt_distdate").toDate(), false);
    _docNumber->setText(q.value("cashrcpt_docnumber").toString());
    _docDate->setDate(q.value("cashrcpt_docdate").toDate(), true);
    _bankaccnt->setId(q.value("cashrcpt_bankaccnt_id").toInt());
    _received->setCurrencyDisabled(true);
    _distDate->setDate(q.value("cashrcpt_distdate").toDate(), true);
    _applDate->setDate(q.value("applydate").toDate(), true);
    _notes->setText(q.value("cashrcpt_notes").toString());
    _posted = q.value("cashrcpt_posted").toBool();
    if(q.value("cashrcpt_salescat_id").toInt() != -1)
    {
      _altAccnt->setChecked(TRUE);
      _salescat->setId(q.value("cashrcpt_salescat_id").toInt());
    }
    if(q.value("cashrcpt_usecustdeposit").toBool())
      _balCustomerDeposit->setChecked(true);
    else
      _balCreditMemo->setChecked(true);

    _origFunds = q.value("cashrcpt_fundstype").toString();
    _fundsType->setCode(_origFunds);
    _cust->setId(q.value("cashrcpt_cust_id").toInt());

    sFillApplyList();
    sFillMiscList();
    setCreditCard();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void cashReceipt::sSearchDocNumChanged()
{
  QString sub = _searchDocNum->text().trimmed();
  if(sub.isEmpty())
    return;

  QList<XTreeWidgetItem*> list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchCaseSensitive, 1);
  if(list.isEmpty())
    list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchStartsWith|Qt::MatchCaseSensitive, 1);

  if(!list.isEmpty())
  {
    _aropen->setCurrentItem(list.at(0));
    _aropen->scrollTo(_aropen->currentIndex());
  }
}

void cashReceipt::sChangeCurrency( int newId)
{
  if (_received->isEnabled())
  {
    XSqlQuery id;
    id.prepare( "UPDATE cashrcpt "
                "SET cashrcpt_curr_id=:curr_id "
                "WHERE (cashrcpt_id=:cashrcpt_id);" );
    id.bindValue(":cashrcpt_id", _cashrcptid);
    id.bindValue(":curr_id", newId);
    id.exec();
    if (id.lastError().type() != QSqlError::NoError)
      systemError(this, id.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void cashReceipt::setCreditCard()
{
  if (! _metrics->boolean("CCAccept"))
    return;

  if (! _fundsTypes[_fundsType->id()].cc)
    return;

  XSqlQuery bankq;
  bankq.prepare("SELECT ccbank_bankaccnt_id"
                "  FROM ccbank"
                " WHERE (ccbank_ccard_type=:cardtype);");
  bankq.bindValue(":cardtype", _fundsType->code());
  bankq.exec();
  if (bankq.first())
    _bankaccnt->setId(bankq.value("ccbank_bankaccnt_id").toInt());
  else if (bankq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, bankq.lastError().text(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::warning(this, tr("No Bank Account"),
                         tr("<p>Cannot find the Bank Account to post this "
                         "transaction against. Either this card type is not "
                         "accepted or the Credit Card configuration is not "
                         "complete."));
    return;
  }

  q.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key)) AS result;");
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":key", omfgThis->_key);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("expireCreditCard", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  MetaSQLQuery mql = mqlLoad("creditCards", "detail");
  ParameterList params;
  params.append("cust_id",    _cust->id());
  params.append("ccard_type", _fundsType->code());
  params.append("masterCard", tr("MasterCard"));
  params.append("visa",       tr("VISA"));
  params.append("americanExpress", tr("American Express"));
  params.append("discover",   tr("Discover"));
  params.append("other",      tr("Other"));
  params.append("key",        omfgThis->_key);
  params.append("activeonly", true);
  q = mql.toQuery(params);
  _cc->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (_cc->topLevelItemCount() == 1)
    _cc->setCurrentItem(_cc->topLevelItem(0));
}

void cashReceipt::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    setCreditCard();

}

void cashReceipt::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    setCreditCard();
}

void cashReceipt::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void cashReceipt::sMoveUp()
{
  q.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();

  setCreditCard();
}

void cashReceipt::sMoveDown()
{
  q.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();

  setCreditCard();
}

void cashReceipt::sDateChanged()
{
  if(_applDate->date() < _mindate)
    _applDate->setDate(_mindate);

  if(_distDate->date() < _applDate->date())
    _applyBalLit->setText(tr("Record Receipt as:"));
  else
    _applyBalLit->setText(tr("Apply Balance As:"));
}
