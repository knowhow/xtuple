/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "arAccountAssignment.h"

#include <QVariant>
#include <QMessageBox>

arAccountAssignment::arAccountAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _araccntid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));

  _customerTypes->setType(XComboBox::CustomerTypes);

  if(!_metrics->boolean("EnableCustomerDeposits"))
  {
    _deferred->hide();
    _deferredLit->hide();
  }

  _ar->setType(GLCluster::cAsset);
  _prepaid->setType(GLCluster::cRevenue);
  _freight->setType(GLCluster::cRevenue | GLCluster::cLiability);
  _deferred->setType(GLCluster::cLiability);
  _discount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

arAccountAssignment::~arAccountAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void arAccountAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arAccountAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("araccnt_id", &valid);
  if (valid)
  {
    _araccntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _customerType->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _customerTypeGroup->setEnabled(FALSE);
      _ar->setReadOnly(TRUE);
      _prepaid->setReadOnly(TRUE);
      _freight->setReadOnly(TRUE);
      _deferred->setReadOnly(TRUE);
      _discount->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void arAccountAssignment::sSave()
{
  if (!_ar->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/R Account Assignment"),
                          tr("You must select a A/R Account before saving this A/R Account Assignment") );
    _ar->setFocus();
    return;
  }

  if (!_prepaid->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/R Account Assignment"),
                          tr("You must select a Prepaid Receivables Account before saving this A/R Account Assignment") );
    _ar->setFocus();
    return;
  }

  if (!_freight->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/R Account Assignment"),
                          tr("You must select a Freight Account before saving this A/R Account Assignment") );
    _freight->setFocus();
    return;
  }

  if(_metrics->boolean("EnableCustomerDeposits") && !_deferred->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/R Account Assignment"),
                          tr("You must select a Deferred Revenue Account before saving this A/R Account Assignment") );
    _deferred->setFocus();
    return;
  }

  if (!_discount->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/R Account Assignment"),
                          tr("You must select a Discount Account before saving this A/R Account Assignment") );
    _discount->setFocus();
    return;
  }

  q.prepare("SELECT araccnt_id"
            "  FROM araccnt"
            " WHERE((araccnt_custtype_id=:araccnt_custtype_id)"
            "   AND (araccnt_custtype=:araccnt_custtype)"
            "   AND (araccnt_id != :araccnt_id))");
  q.bindValue(":araccnt_id", _araccntid);
  if (_selectedCustomerType->isChecked())
  {
    q.bindValue(":araccnt_custtype_id", _customerTypes->id());
    q.bindValue(":araccnt_custtype", "^[a-zA-Z0-9_]");
  }
  else if (_customerTypePattern->isChecked())
  {
    q.bindValue(":araccnt_custtype_id", -1);
    q.bindValue(":araccnt_custtype", _customerType->text());
  }
  q.exec();
  if(q.first())
  {
    QMessageBox::warning( this, tr("Cannot Save A/R Account Assignment"),
      tr("A record for that assignment already exists."));
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('araccnt_araccnt_id_seq') AS _araccntid;");
    if (q.first())
      _araccntid = q.value("_araccntid").toInt();
//  ToDo

    q.prepare( "INSERT INTO araccnt "
               "( araccnt_id, araccnt_custtype_id, araccnt_custtype,"
               "  araccnt_ar_accnt_id, araccnt_prepaid_accnt_id, araccnt_freight_accnt_id,"
               "  araccnt_deferred_accnt_id, araccnt_discount_accnt_id ) "
               "VALUES "
               "( :araccnt_id, :araccnt_custtype_id, :araccnt_custtype,"
               "  :araccnt_ar_accnt_id, :araccnt_prepaid_accnt_id, :araccnt_freight_accnt_id,"
               "  :araccnt_deferred_accnt_id, :araccnt_discount_accnt_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE araccnt "
               "SET araccnt_custtype_id=:araccnt_custtype_id, araccnt_custtype=:araccnt_custtype,"
               "    araccnt_ar_accnt_id=:araccnt_ar_accnt_id,"
               "    araccnt_prepaid_accnt_id=:araccnt_prepaid_accnt_id,"
               "    araccnt_freight_accnt_id=:araccnt_freight_accnt_id,"
               "    araccnt_deferred_accnt_id=:araccnt_deferred_accnt_id,"
			   "    araccnt_discount_accnt_id=:araccnt_discount_accnt_id "
               "WHERE (araccnt_id=:araccnt_id);" );

  q.bindValue(":araccnt_id", _araccntid);

  if (_selectedCustomerType->isChecked())
  {
    q.bindValue(":araccnt_custtype_id", _customerTypes->id());
    q.bindValue(":araccnt_custtype", "^[a-zA-Z0-9_]");
  }
  else if (_customerTypePattern->isChecked())
  {
    q.bindValue(":araccnt_custtype_id", -1);
    q.bindValue(":araccnt_custtype", _customerType->text());
  }

  q.bindValue(":araccnt_ar_accnt_id", _ar->id());
  q.bindValue(":araccnt_prepaid_accnt_id", _prepaid->id());
  q.bindValue(":araccnt_freight_accnt_id", _freight->id());
  q.bindValue(":araccnt_deferred_accnt_id", _deferred->id());
  q.bindValue(":araccnt_discount_accnt_id", _discount->id());

  q.exec();

  done(_araccntid);
}

void arAccountAssignment::populate()
{
  q.prepare( "SELECT araccnt_custtype_id, araccnt_custtype,"
             "       araccnt_ar_accnt_id, araccnt_prepaid_accnt_id,"
             "       araccnt_freight_accnt_id, araccnt_deferred_accnt_id, "
			 "       araccnt_discount_accnt_id "
             "FROM araccnt "
             "WHERE (araccnt_id=:araccnt_id);" );
  q.bindValue(":araccnt_id", _araccntid);
  q.exec();
  if (q.first())
  {
    if (q.value("araccnt_custtype_id").toInt() == -1)
    {
      _customerTypePattern->setChecked(TRUE);
      _customerType->setText(q.value("araccnt_custtype").toString());
    }
    else
    {
      _selectedCustomerType->setChecked(TRUE);
      _customerTypes->setId(q.value("araccnt_custtype_id").toInt());
    }

    _ar->setId(q.value("araccnt_ar_accnt_id").toInt());
    _prepaid->setId(q.value("araccnt_prepaid_accnt_id").toInt());
    _freight->setId(q.value("araccnt_freight_accnt_id").toInt());
    _deferred->setId(q.value("araccnt_deferred_accnt_id").toInt());
    _discount->setId(q.value("araccnt_discount_accnt_id").toInt());
  }
}
