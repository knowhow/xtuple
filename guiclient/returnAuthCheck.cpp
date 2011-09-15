/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnAuthCheck.h"
#include "storedProcErrorLookup.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

returnAuthCheck::returnAuthCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_bankaccnt, SIGNAL(newID(int)),    this, SLOT(sPopulateBankInfo(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _date->setDate(omfgThis->dbDate(), true);

  _bankaccnt->setAllowNull(TRUE);
  _bankaccnt->setType(XComboBox::APBankAccounts);
  _cmheadcurrid = CurrDisplay::baseId();
}

returnAuthCheck::~returnAuthCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnAuthCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnAuthCheck::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  param = pParams.value("cmhead_id", &valid);
  if (valid)
    _cmheadid=param.toInt();

  populate();

  return NoError;
}

void returnAuthCheck::sSave()
{
  if (!_date->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter a date for this check.") );
    _date->setFocus();
    return;
  }

  else if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter an amount for this check.") );
    return;
  }

  else if (!_bankaccnt->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must select a bank account for this check.") );
    _date->setFocus();
    return;
  }

  else
  {
    q.prepare("SELECT createCheck(:bankaccnt_id, 'C', :recipid,"
	      "                   :checkDate, :amount, :curr_id, NULL,"
	      "                   NULL, :for, :notes, TRUE, :aropen_id) AS result; ");
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.bindValue(":recipid",	_custid);
    q.bindValue(":checkDate", _date->date());
    q.bindValue(":amount",	_amount->localValue());
    q.bindValue(":curr_id",	_amount->id());
    q.bindValue(":for",	_for->text().trimmed());
    q.bindValue(":notes", _notes->toPlainText().trimmed());
	q.bindValue(":aropen_id", _aropenid);
	q.exec();
    if (q.first())
    {
      _checkid = q.value("result").toInt();
      if (_checkid < 0)
      {
        systemError(this, storedProcErrorLookup("createCheck", _checkid),
		    __FILE__, __LINE__);
        return;
      }
      q.prepare( "SELECT checkhead_number "
               "FROM checkhead "
               "WHERE (checkhead_id=:check_id);" );
      q.bindValue(":check_id", _checkid);
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
	  done(TRUE);
	}
    else if (q.lastError().type() != QSqlError::NoError)
    {
     systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
    }
  }
}

void returnAuthCheck::sClose()
{
  done(-1);
}

void returnAuthCheck::sPopulateBankInfo(int pBankaccntid)
{
  if ( pBankaccntid == -1 )
  {
    _amount->setId(_cmheadcurrid);
    _checkNum->clear();
  }
  else
  {
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT bankaccnt_nextchknum, bankaccnt_curr_id "
			 "FROM bankaccnt "
			 "WHERE (bankaccnt_id=:bankaccnt_id);" );
    checkNumber.bindValue(":bankaccnt_id", pBankaccntid);
    checkNumber.exec();
    if (checkNumber.first())
    {
      if (checkNumber.value("bankaccnt_curr_id").toInt() != _cmheadcurrid)
      {
	QMessageBox::critical(this, tr("Bank Uses Different Currency"),
			      tr("<p>The currency of the bank account is not "
				 "the same as the currency of the credit "
				 "memo. You may not use this bank account."));
	_amount->setId(_cmheadcurrid);
	_bankaccnt->setId(-1);
	return;
      }
      _checkNum->setText(checkNumber.value("bankaccnt_nextchknum").toString());
      _amount->setId(checkNumber.value("bankaccnt_curr_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, checkNumber.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthCheck::populate()
{
  q.prepare("SELECT cust_id,cust_name,cmhead_number,cmhead_curr_id, "
	        "'Return Authorization ' || rahead_number::text AS memo, "
			"'Applied Against Credit Memo ' || cmhead_number::text AS note, "
			"aropen_id,aropen_amount "
			"FROM rahead,cmhead,custinfo,aropen "
			"WHERE ((cmhead_cust_id=cust_id) "
			"AND (rahead_id=cmhead_rahead_id) "
			"AND (cmhead_id=:cmhead_id) "
			"AND (aropen_doctype='C') "
			"AND (aropen_docnumber=cmhead_number));");
  q.bindValue(":cmhead_id",_cmheadid);
  q.exec();
  if (q.first())
  {
    _custid=q.value("cust_id").toInt();
	_aropenid=q.value("aropen_id").toInt();
    _custName->setText(q.value("cust_name").toString());
	_creditmemo->setText(q.value("cmhead_number").toString());
	_cmheadcurrid = q.value("cmhead_curr_id").toInt();
	_amount->setId(q.value("cmhead_curr_id").toInt());
	_amount->setLocalValue(q.value("aropen_amount").toDouble());
	_for->setText(q.value("memo").toString());
	_notes->setText(q.value("note").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT bankaccnt_id "
	    "FROM bankaccnt "
	    "WHERE (bankaccnt_ap"
	    "  AND  (bankaccnt_type='K')"
	    "  AND  (bankaccnt_curr_id=:cmcurrid));");
  q.bindValue(":cmcurrid", _cmheadcurrid);
  q.exec();
  if (q.first())
    _bankaccnt->setId(q.value("bankaccnt_id").toInt());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _bankaccnt->setId(-1);
}
