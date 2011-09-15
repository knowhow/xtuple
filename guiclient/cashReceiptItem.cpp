/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cashReceiptItem.h"
#include "applyARDiscount.h"

#include <QVariant>
#include <QValidator>
#include <QMessageBox>

/*
 *  Constructs a cashReceiptItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
cashReceiptItem::cashReceiptItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_discount, SIGNAL(clicked()),      this, SLOT(sDiscount()));

  _cust->setReadOnly(TRUE);
  adjustSize();
}

/*
 *  Destroys the object and frees any allocated resources
 */
cashReceiptItem::~cashReceiptItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void cashReceiptItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceiptItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
    _openAmount->setId(param.toInt());

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
    _cashrcptid = param.toInt();
  else
    _cashrcptid = -1;

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    populate();
  }
  else
    _aropenid = -1;

  param = pParams.value("cashrcptitem_id", &valid);
  if (valid)
  {
    _cashrcptitemid = param.toInt();
    populate();
  }
  else
    _cashrcptitemid = -1;

  param = pParams.value("amount_to_apply", &valid);
  if (valid)
  {
    _amttoapply = param.toDouble();
	populate();
  }

  return NoError;
}

void cashReceiptItem::sSave()
{
  if (_amountToApply->localValue() > _openAmount->localValue())
  {
    QMessageBox::warning( this, tr("Cannot Apply"),
      tr("You may not apply more than the balance of this item.") );
    _amountToApply->setFocus();
    return;
  }

  int sense = 1;
  if (_docType->text() == "C" || _docType->text() == "R")
    sense = -1;

  if (_mode == cNew)
  {
    XSqlQuery cashrcptitemid("SELECT NEXTVAL('cashrcptitem_cashrcptitem_id_seq') AS _cashrcptitem_id;");
    if (cashrcptitemid.first())
      _cashrcptitemid = cashrcptitemid.value("_cashrcptitem_id").toInt();

    XSqlQuery newReceipt;
    newReceipt.prepare( "INSERT INTO cashrcptitem "
                        "( cashrcptitem_id, cashrcptitem_cashrcpt_id,"
                        "  cashrcptitem_aropen_id, cashrcptitem_amount, cashrcptitem_discount ) "
                        "VALUES "
                        "( :cashrcptitem_id, :cashrcptitem_cashrcpt_id,"
                        "  :cashrcptitem_aropen_id, :cashrcptitem_amount, :cashrcptitem_discount );" );
    newReceipt.bindValue(":cashrcptitem_id", _cashrcptitemid);
    newReceipt.bindValue(":cashrcptitem_cashrcpt_id", _cashrcptid);
    newReceipt.bindValue(":cashrcptitem_aropen_id", _aropenid);
    newReceipt.bindValue(":cashrcptitem_amount", _amountToApply->localValue() * sense);
    newReceipt.bindValue(":cashrcptitem_discount", _discountAmount->localValue() * sense);

    newReceipt.exec();
  }
  else if (_mode == cEdit)
  {
    XSqlQuery updateReceipt;
    updateReceipt.prepare( "UPDATE cashrcptitem "
                           "SET cashrcptitem_amount=:cashrcptitem_amount, "
						   "cashrcptitem_discount=:cashrcptitem_discount "
                           "WHERE (cashrcptitem_id=:cashrcptitem_id);" );
    updateReceipt.bindValue(":cashrcptitem_id", _cashrcptitemid);
    updateReceipt.bindValue(":cashrcptitem_amount", _amountToApply->localValue() * sense);
    updateReceipt.bindValue(":cashrcptitem_discount", _discountAmount->localValue() * sense);

    updateReceipt.exec();
  }

  done(_cashrcptitemid);
}

void cashReceiptItem::populate()
{
  XSqlQuery query;

  if (_mode == cNew)
  {
    query.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_doctype, "
                   "       aropen_docdate, aropen_duedate, "
                   "       cashrcpt_curr_id, cashrcpt_distdate, "
                   "       currToCurr(aropen_curr_id, cashrcpt_curr_id, (aropen_amount - aropen_paid), "
                   "       cashrcpt_distdate) AS f_amount, "
                   "       COALESCE(ABS(cashrcptitem_discount), 0.00) AS discount "
                   "FROM cashrcpt, aropen LEFT OUTER JOIN cashrcptitem ON (aropen_id=cashrcptitem_aropen_id) "
                   "WHERE ( (aropen_id=:aropen_id)"
                   " AND (cashrcpt_id=:cashrcpt_id))" );
    query.bindValue(":aropen_id", _aropenid);
    query.bindValue(":cashrcpt_id", _cashrcptid);
    query.exec();
    if (query.first())
    {
      _cust->setId(query.value("aropen_cust_id").toInt());
      _docNumber->setText(query.value("aropen_docnumber").toString());
      _docType->setText(query.value("aropen_doctype").toString());
      _docDate->setDate(query.value("aropen_docdate").toDate(), true);
      _dueDate->setDate(query.value("aropen_duedate").toDate());
      _discountAmount->setLocalValue(query.value("discount").toDouble());
      _openAmount->set(query.value("f_amount").toDouble(),
                       query.value("cashrcpt_curr_id").toInt(),
                       query.value("cashrcpt_distdate").toDate(), false);
      _discount->setEnabled(query.value("aropen_doctype").toString() == "I" ||
                            query.value("aropen_doctype").toString() == "D");
    } 
  }
  else if (_mode == cEdit)
  {
    query.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_doctype, "
                   "       aropen_docdate, aropen_duedate, "
                   "       currToCurr(aropen_curr_id, cashrcpt_curr_id, (aropen_amount - aropen_paid), "
                   "       cashrcpt_distdate) AS balance, "
                   "       ABS(cashrcptitem_amount) AS cashrcptitem_amount, "
                   "       cashrcpt_curr_id, cashrcpt_distdate,  "
                   "       ABS(cashrcptitem_discount) AS discount "
                   "FROM cashrcptitem, cashrcpt, aropen "
                   "WHERE ( (cashrcptitem_cashrcpt_id=cashrcpt_id)"
                   " AND (cashrcptitem_aropen_id=aropen_id)"
                   " AND (cashrcptitem_id=:cashrcptitem_id) );" );
    query.bindValue(":cashrcptitem_id", _cashrcptitemid);
    query.exec();
    if (query.first())
    {
      _cust->setId(query.value("aropen_cust_id").toInt());
      _docNumber->setText(query.value("aropen_docnumber").toString());
      _docType->setText(query.value("aropen_doctype").toString());
      _docDate->setDate(query.value("aropen_docdate").toDate(), true);
      _dueDate->setDate(query.value("aropen_duedate").toDate());
      _openAmount->set(query.value("balance").toDouble(),
		       query.value("cashrcpt_curr_id").toInt(),
                       query.value("cashrcpt_distdate").toDate(), false);
      _amountToApply->setLocalValue(query.value("cashrcptitem_amount").toDouble());
      _discountAmount->setLocalValue(query.value("discount").toDouble());
      _discount->setEnabled(query.value("aropen_doctype").toString() == "I" ||
                            query.value("aropen_doctype").toString() == "D");
    }
//  ToDo
  }
}

void cashReceiptItem::sDiscount()
{
  ParameterList params;
  params.append("aropen_id", _aropenid);
  params.append("curr_id", _openAmount->id());
  if(_discountAmount->localValue() != 0.0)
    params.append("amount", _discountAmount->localValue());

  applyARDiscount newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() != XDialog::Rejected)
  {
    _discountAmount->setLocalValue(newdlg._amount->localValue());
    _amountToApply->setLocalValue(_openAmount->localValue() - _discountAmount->localValue());
  }
}
