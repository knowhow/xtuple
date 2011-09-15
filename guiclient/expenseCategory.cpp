/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "expenseCategory.h"

#include <QVariant>
#include <QMessageBox>

expenseCategory::expenseCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _expcatid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_category, SIGNAL(lostFocus()), this, SLOT(sCheck()));

  _expense->setType(GLCluster::cExpense | GLCluster::cAsset | GLCluster::cLiability);
  _purchasePrice->setType(GLCluster::cAsset | GLCluster::cExpense);
  _liability->setType(GLCluster::cLiability);
  _freight->setType(GLCluster::cExpense);

}

expenseCategory::~expenseCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void expenseCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse expenseCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("expcat_id", &valid);
  if (valid)
  {
    _expcatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _category->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _category->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
      _category->clear();
      _category->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _expense->setReadOnly(TRUE);
      _purchasePrice->setReadOnly(TRUE);
      _liability->setReadOnly(TRUE);
      _freight->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void expenseCategory::sCheck()
{
  _category->setText(_category->text().trimmed().toUpper());
  if ( ((_mode == cNew) || (_mode == cCopy)) && (_category->text().length() != 0) )
  {
    q.prepare( "SELECT expcat_id "
               "FROM expcat "
               "WHERE (UPPER(expcat_code)=:expcat_code);" );
    q.bindValue(":expcat_code", _category->text().trimmed());
    q.exec();
    if (q.first())
    {
      _expcatid = q.value("expcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
  }
}

void expenseCategory::sSave()
{
  if(_category->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
      tr("You must specify a name."));
    _category->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "SELECT expcat_id "
               "  FROM expcat "
               " WHERE((UPPER(expcat_code)=UPPER(:expcat_code))"
               "   AND (expcat_id != :expcat_id));" );
    q.bindValue(":expcat_id", _expcatid);
    q.bindValue(":expcat_code", _category->text().trimmed());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                                  tr("The name you have specified is already in use."));
      _category->setFocus();
      return;
    }
  }

  if (!_expense->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a Expense Account Number for this Expense Category before you may save it.") );
    _expense->setFocus();
    return;
  }

  if (!_purchasePrice->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a Purchase Price Variance Account Number for this Expense Category before you may save it.") );
    _purchasePrice->setFocus();
    return;
  }

  if (!_liability->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a P/O Liability Clearing Account Number for this Expense Category before you may save it.") );
    _expense->setFocus();
    return;
  }

  if (!_freight->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a Freight Receiving Account Number for this Expense Category before you may save it.") );
    _freight->setFocus();
    return;
  }

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    q.exec("SELECT NEXTVAL('expcat_expcat_id_seq') AS expcat_id");
    if (q.first())
      _expcatid = q.value("expcat_id").toInt();

    q.prepare( "INSERT INTO expcat"
               "( expcat_id, expcat_code, expcat_active, expcat_descrip,"
               "  expcat_exp_accnt_id, expcat_purchprice_accnt_id,"
               "  expcat_liability_accnt_id, expcat_freight_accnt_id ) "
               "VALUES "
               "( :expcat_id, :expcat_code, :expcat_active, :expcat_descrip,"
               "  :expcat_exp_accnt_id, :expcat_purchprice_accnt_id,"
               "  :expcat_liability_accnt_id, :expcat_freight_accnt_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE expcat "
               "SET expcat_code=:expcat_code, expcat_active=:expcat_active,"
               "    expcat_descrip=:expcat_descrip,"
               "    expcat_exp_accnt_id=:expcat_exp_accnt_id,"
               "    expcat_purchprice_accnt_id=:expcat_purchprice_accnt_id,"
               "    expcat_liability_accnt_id=:expcat_liability_accnt_id,"
               "    expcat_freight_accnt_id=:expcat_freight_accnt_id "
               "WHERE (expcat_id=:expcat_id);" );

  q.bindValue(":expcat_id", _expcatid);
  q.bindValue(":expcat_code", _category->text().trimmed());
  q.bindValue(":expcat_active", QVariant(_active->isChecked()));
  q.bindValue(":expcat_descrip", _description->text().trimmed());
  q.bindValue(":expcat_exp_accnt_id", _expense->id());
  q.bindValue(":expcat_purchprice_accnt_id", _purchasePrice->id());
  q.bindValue(":expcat_liability_accnt_id", _liability->id());
  q.bindValue(":expcat_freight_accnt_id", _freight->id());
  q.exec();

  done(_expcatid);
}

void expenseCategory::populate()
{
  q.prepare( "SELECT * "
             "FROM expcat "
             "WHERE (expcat_id=:expcat_id);" );
  q.bindValue(":expcat_id", _expcatid);
  q.exec();
  if (q.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(q.value("expcat_code").toString());
      _description->setText(q.value("expcat_descrip").toString());
      _active->setChecked(q.value("expcat_active").toBool());
    }
    else
      _active->setChecked(TRUE);

    _expense->setId(q.value("expcat_exp_accnt_id").toInt());
    _purchasePrice->setId(q.value("expcat_purchprice_accnt_id").toInt());
    _liability->setId(q.value("expcat_liability_accnt_id").toInt());
    _freight->setId(q.value("expcat_freight_accnt_id").toInt());
  }
}
 
