/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "miscCheck.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

miscCheck::miscCheck(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)),    this, SLOT(sPopulateBankInfo(int)));
  connect(_cust,      SIGNAL(newId(int)),    this, SLOT(sHandleButtons()));
  connect(_custRB,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_save,      SIGNAL(clicked()),     this, SLOT(sSave()));
  connect(_taxauth,   SIGNAL(newID(int)),    this, SLOT(sHandleButtons()));
  connect(_taxauthRB, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_vend,      SIGNAL(newId(int)),    this, SLOT(sHandleButtons()));
  connect(_vendRB,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_cust,      SIGNAL(valid(bool)),   this, SLOT(sCustomerSelected()));

  _captive = FALSE;
  _raheadid = -1;
  _aropenamt = 0;

  _bankaccnt->setType(XComboBox::APBankAccounts);
}

miscCheck::~miscCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void miscCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse miscCheck::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());
   
  param = pParams.value("check_id", &valid);
  if (valid)
  {
    connect(_cmCluster, SIGNAL(newId(int)),   this, SLOT(sCreditMemoSelected()));
    _checkid = param.toInt();
    populate();
  }

  if (pParams.inList("new"))
  {
    _mode = cNew;

    _save->setText(tr("C&reate"));

    _vend->setFocus();
    
    connect(_cmCluster, SIGNAL(newId(int)),   this, SLOT(sCreditMemoSelected()));
  }
  else if (pParams.inList("edit"))
  {
    _mode = cEdit;

    _bankaccnt->setEnabled(FALSE);

    _save->setFocus();
  }

  return NoError;
}

void miscCheck::sSave()
{
  double _amt;
  _amt=_amount->localValue();
  XSqlQuery check;
  if (!_date->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter a date for this check.") );
    _date->setFocus();
    return;
  }
  
  if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter an amount for this check.") );
    _date->setFocus();
    return;
  }

  if ( (_applytocm->isChecked()) && (_cmCluster->id() == -1) )
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must select a Credit Memo for this "
			     "expensed check.") );
    _expcat->setFocus();
    return;
  }
 
  if (_applytocm->isChecked() && _cmCluster->isValid() && (_amt > _aropenamt))
  {
    QMessageBox::warning( this, tr("Invalid Amount"),
                            tr("<p>You must enter an amount less than or equal to the  "
	   		                   "credit memo selected.") );
    _amount->setLocalValue(_aropenamt);
	_amount->setFocus();
	return;
  }
 
  if ( (_expense->isChecked()) && (_expcat->id() == -1) )
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must select an Expense Category for this "
			     "expensed check.") );
    _expcat->setFocus();
    return;
  }
 
  if (_mode == cNew)
  {
    check.prepare("SELECT createCheck(:bankaccnt_id, :reciptype, :recipid,"
	      "                   :checkDate, :amount, :curr_id, :expcat_id,"
		  "                   NULL, :for, :notes, TRUE, :cmhead_id) AS result;" );
    check.bindValue(":bankaccnt_id", _bankaccnt->id());
  }
  else if (_mode == cEdit)
  {
    check.prepare( "UPDATE checkhead "
               "SET checkhead_recip_id=:recipid,"
	       "    checkhead_recip_type=:reciptype,"
	       "    checkhead_checkdate=:checkDate, "
	       "    checkhead_amount=:amount, checkhead_curr_id=:curr_id, "
               "    checkhead_expcat_id=:expcat_id, checkhead_for=:for,"
	       "    checkhead_notes=:notes "
               "WHERE (checkhead_id=:check_id);" );
    check.bindValue(":check_id", _checkid);
  }

  if (_vendRB->isChecked())
  {
    check.bindValue(":reciptype",	"V");
    check.bindValue(":recipid",	_vend->id());
  }
  else if (_custRB->isChecked())
  {
    check.bindValue(":reciptype",	"C");
    check.bindValue(":recipid",	_cust->id());
  }
  else if (_taxauthRB->isChecked())
  {
    check.bindValue(":reciptype",	"T");
    check.bindValue(":recipid",	_taxauth->id());
  }
  check.bindValue(":checkDate",	_date->date());
  check.bindValue(":amount",	_amt);
  check.bindValue(":curr_id",	_amount->id());
  check.bindValue(":for",		_for->text().trimmed());
  check.bindValue(":notes",		_notes->toPlainText().trimmed());

  if (_expense->isChecked())
    check.bindValue(":expcat_id", _expcat->id());

  if ( (_applytocm->isChecked()) && (_cmCluster->isValid()) )
	check.bindValue(":cmhead_id",_cmCluster->id());

  check.exec();
  if (_mode == cNew && check.first())
  {
    _checkid = check.value("result").toInt();
    if (_checkid < 0)
    {
      systemError(this, storedProcErrorLookup("createCheck", _checkid),
		  __FILE__, __LINE__);
      return;
    }
    check.prepare( "SELECT checkhead_number "
               "FROM checkhead "
               "WHERE (checkhead_id=:check_id);" );
    check.bindValue(":check_id", _checkid);
    check.exec();
    if (check.first())
    {
// Since we are no longer assigning check number at creation this is not needed
/*
      QMessageBox::information( this, tr("New Check Created"),
                                tr("<p>A new Check has been created and "
				   "assigned #%1")
                                .arg(check.value("checkhead_number").toString()) );
*/
    }
    else if (check.lastError().type() != QSqlError::NoError)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (check.lastError().type() != QSqlError::NoError)
  {
    systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sChecksUpdated(_bankaccnt->id(), _checkid, TRUE);

  if (_captive)
    close();
}

void miscCheck::sPopulateBankInfo(int pBankaccntid)
{
  if ( pBankaccntid != -1 )
  {
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT bankaccnt_nextchknum, bankaccnt_curr_id "
			 "FROM bankaccnt "
			 "WHERE (bankaccnt_id=:bankaccnt_id);" );
    checkNumber.bindValue(":bankaccnt_id", pBankaccntid);
    checkNumber.exec();
    if (checkNumber.first())
    {
      _amount->setId(checkNumber.value("bankaccnt_curr_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, checkNumber.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_cmCluster->isValid())
    sCreditMemoSelected();
}

void miscCheck::populate()
{
  q.prepare( "SELECT checkhead_recip_type, checkhead_recip_id,"
	     "       checkhead_bankaccnt_id, checkhead_number,"
             "       checkhead_checkdate, checkhead_expcat_id,"
             "       checkhead_for, checkhead_notes,"
             "       checkhead_amount, checkhead_curr_id, aropen_id AS checkhead_cmhead_id "
             "FROM checkhead "
             "  LEFT OUTER JOIN checkitem ON (checkhead_id=checkitem_checkhead_id) "
             "                            AND (checkitem_aropen_id IS NOT NULL) "
             "  LEFT OUTER JOIN aropen ON (checkitem_aropen_id=aropen_id) "
             "                         AND (aropen_doctype = 'C') "
             "WHERE checkhead_id=:check_id;");
  q.bindValue(":check_id", _checkid);
  q.exec();
  if (q.first())
  {
    if (q.value("checkhead_recip_type").toString() == "V")
    {
      _vendRB->setChecked(true);
      _vend->setId(q.value("checkhead_recip_id").toInt());
    }
    else if (q.value("checkhead_recip_type").toString() == "C")
    {
      _custRB->setChecked(true);
      _cust->setId(q.value("checkhead_recip_id").toInt());
    }
    else if (q.value("checkhead_recip_type").toString() == "T")
    {
      _taxauthRB->setChecked(true);
      _taxauth->setId(q.value("checkhead_recip_id").toInt());
    }
    // bank accnt must be set before check number and currency
    _bankaccnt->setId(q.value("checkhead_bankaccnt_id").toInt());
    _date->setDate(q.value("checkhead_checkdate").toDate(), true);
    _amount->set(q.value("checkhead_amount").toDouble(),
		 q.value("checkhead_curr_id").toInt(),
		 q.value("checkhead_checkdate").toDate(), false);
    _for->setText(q.value("checkhead_for"));
    _notes->setText(q.value("checkhead_notes").toString());

    if (!q.value("checkhead_cmhead_id").isNull())
    {
      _applytocm->setChecked(TRUE);
      _cmCluster->setId(q.value("checkhead_cmhead_id").toInt());
    }
    else if (q.value("checkhead_expcat_id").isNull() ||
	q.value("checkhead_expcat_id").toInt() == -1)
      _memo->setChecked(TRUE);
    else
    {
      _expense->setChecked(TRUE);
      _expcat->setId(q.value("checkhead_expcat_id").toInt());
    }

    if (!q.value("checkhead_cmhead_id").isNull())
    {
      _recipGroup->setEnabled(FALSE);
      _chargeToGroup->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _for->setEnabled(FALSE);
    }
  }
}

void miscCheck::sHandleButtons()
{
  if (_vendRB->isChecked())
  {
    _widgetStack->setCurrentIndex(0);
	_memo->setText("Create Credit Memo");
  }
  else if (_custRB->isChecked())
  {
	_widgetStack->setCurrentIndex(1);
	_memo->setText("Create Debit Memo");
	_cmCluster->setId(-1);
	_cmCluster->setCustId(_cust->id());
  }
  else
  {
	_widgetStack->setCurrentIndex(2);
	_memo->setText("Create Credit Memo");
  }

  _save->setEnabled((_cust->isValid() && _custRB->isChecked())
	             || (_vend->isValid()  && _vendRB->isChecked()) 
				 || (_taxauth->id() > 0 && _taxauthRB->isChecked()));
}

void miscCheck::sCustomerSelected()
{
  _cmCluster->setEnabled(_cust->isValid() && _applytocm->isChecked());
}

void miscCheck::sCreditMemoSelected()
{
  if (_cmCluster->id() != -1) 
  {  
	if(!_date->isValid())
	  _date->setDate(QDate::currentDate());
	q.prepare("SELECT aropen_curr_id, "
			  "       round((aropen_amount-aropen_paid- "
			  //Subtract amount for existing checks
			  "(SELECT COALESCE(SUM(checkhead_amount),0) "
			  " FROM checkhead,checkitem "
			  " WHERE ((checkhead_id=checkitem_checkhead_id) "
			  " AND (NOT checkhead_posted) "
			  " AND (NOT checkhead_void) "
                          " AND (checkhead_id <> :check_id) "
			  " AND (checkitem_aropen_id=aropen_id))) "

			  ") * aropen_curr_rate / currRate(:curr_id,aropen_docdate),2) AS amount "
              "  FROM aropen "
              " WHERE (aropen_id=:aropen_id); ");
    q.bindValue(":aropen_id", _cmCluster->id());
    q.bindValue(":curr_id", _amount->id());
    q.bindValue(":date", _date->date());
    q.bindValue(":check_id", _checkid);
    q.exec();
    if (q.first())
    {
      _aropenamt=q.value("amount").toDouble();
      if (_mode == cNew)
        _amount->setLocalValue(q.value("amount").toDouble());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    } 
  }
  else
  {
    _aropenamt=0;
  } 
}
