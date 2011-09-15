/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cashReceiptMiscDistrib.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

cashReceiptMiscDistrib::cashReceiptMiscDistrib(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  adjustSize();
}

cashReceiptMiscDistrib::~cashReceiptMiscDistrib()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceiptMiscDistrib::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceiptMiscDistrib::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
    _cashrcptid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("cashrcptmisc_id", &valid);
  if (valid)
  {
    _cashrcptmiscid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
  }

  return NoError;
}

void cashReceiptMiscDistrib::populate()
{
  q.prepare( "SELECT cashrcptmisc_accnt_id, cashrcptmisc_notes,"
             "       cashrcptmisc_amount, cashrcpt_curr_id, cashrcpt_distdate "
             "FROM cashrcptmisc JOIN cashrcpt ON (cashrcptmisc_cashrcpt_id = cashrcpt_id) "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  q.bindValue(":cashrcptmisc_id", _cashrcptmiscid);
  q.exec();
  if (q.first())
  {
    _account->setId(q.value("cashrcptmisc_accnt_id").toInt());
    _amount->set(q.value("cashrcptmisc_amount").toDouble(),
    		 q.value("cashrcpt_curr_id").toInt(),
		 q.value("cashrcpt_distdate").toDate(), false);
    _notes->setText(q.value("cashrcptmisc_notes").toString());
  }
}

void cashReceiptMiscDistrib::sSave()
{
  if (!_account->isValid())
  {
    QMessageBox::warning( this, tr("Select Account"),
                          tr("You must select an Account to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Enter Amount"),
                          tr("You must enter an amount for this Miscellaneous Distribution.") );
    _amount->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('cashrcptmisc_cashrcptmisc_id_seq') AS _cashrcptmisc_id;");
    if (q.first())
      _cashrcptmiscid = q.value("_cashrcptmisc_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO cashrcptmisc "
               "( cashrcptmisc_id, cashrcptmisc_cashrcpt_id,"
               "  cashrcptmisc_accnt_id, cashrcptmisc_amount,"
               "  cashrcptmisc_notes ) "
               "VALUES "
               "( :cashrcptmisc_id, :cashrcptmisc_cashrcpt_id,"
               "  :cashrcptmisc_accnt_id, :cashrcptmisc_amount,"
               "  :cashrcptmisc_notes );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE cashrcptmisc "
               "SET cashrcptmisc_accnt_id=:cashrcptmisc_accnt_id,"
               "    cashrcptmisc_amount=:cashrcptmisc_amount, cashrcptmisc_notes=:cashrcptmisc_notes "
               "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );

  q.bindValue(":cashrcptmisc_id", _cashrcptmiscid);
  q.bindValue(":cashrcptmisc_cashrcpt_id", _cashrcptid);
  q.bindValue(":cashrcptmisc_accnt_id", _account->id());
  q.bindValue(":cashrcptmisc_amount", _amount->localValue());
  q.bindValue(":cashrcptmisc_notes",       _notes->toPlainText().trimmed());
  q.exec();

  done(_cashrcptmiscid);
}

