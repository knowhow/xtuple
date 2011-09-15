/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apAccountAssignment.h"

#include <QVariant>
#include <QMessageBox>

apAccountAssignment::apAccountAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_selectedVendorType, SIGNAL(toggled(bool)), _vendorTypes, SLOT(setEnabled(bool)));
  connect(_vendorTypePattern, SIGNAL(toggled(bool)), _vendorType, SLOT(setEnabled(bool)));

  _vendorTypes->setType(XComboBox::VendorTypes);

  _ap->setType(GLCluster::cLiability);
  _prepaid->setType(GLCluster::cAsset);
  _discount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

apAccountAssignment::~apAccountAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void apAccountAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apAccountAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("apaccnt_id", &valid);
  if (valid)
  {
    _apaccntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _vendorType->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vendorTypeGroup->setEnabled(FALSE);
      _ap->setReadOnly(TRUE);
      _prepaid->setReadOnly(TRUE);
      _discount->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void apAccountAssignment::sSave()
{
  if (!_ap->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/P Account Assignment"),
                          tr("You must select an A/P Account before saving this A/P Account Assignment") );
    _ap->setFocus();
    return;
  }

  if (!_prepaid->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/P Account Assignment"),
                          tr("You must select a Prepaid Account before saving this A/P Account Assignment") );
    _prepaid->setFocus();
    return;
  }

  if (!_discount->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/P Account Assignment"),
                          tr("You must select a Discount Account before saving this A/P Account Assignment") );
    _discount->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT apaccnt_id "
               "FROM apaccnt "
               "WHERE ( (apaccnt_vendtype_id=:apaccnt_vendtype_id)"
               " AND (apaccnt_vendtype=:apaccnt_vendtype) );" );

    if (_allVendorTypes->isChecked())
    {
      q.bindValue(":apaccnt_vendtype_id", -1);
      q.bindValue(":apaccnt_vendtype", ".*");
    }
    else if (_selectedVendorType->isChecked())
    {
      q.bindValue(":apaccnt_vendtype_id", _vendorTypes->id());
      q.bindValue(":apaccnt_vendtype", "");
    }
    else if (_vendorTypePattern->isChecked())
    {
      q.bindValue(":apaccnt_vendtype_id", -1);
      q.bindValue(":apaccnt_vendtype", _vendorType->text().trimmed());
    }

    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create A/P Account Assignment"),
                             tr("You may not save this A/P Account Assignment as it already exists.") );
      return;
    }

    q.exec("SELECT NEXTVAL('apaccnt_apaccnt_id_seq') AS _apaccnt_id;");
    if (q.first())
      _apaccntid = q.value("_apaccnt_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO apaccnt "
               "( apaccnt_id, apaccnt_vendtype_id, apaccnt_vendtype,"
               "  apaccnt_ap_accnt_id, apaccnt_prepaid_accnt_id,"
               "  apaccnt_discount_accnt_id ) "
               "VALUES "
               "( :apaccnt_id, :apaccnt_vendtype_id, :apaccnt_vendtype,"
               "  :apaccnt_ap_accnt_id, :apaccnt_prepaid_accnt_id,"
               "  :apaccnt_discount_accnt_id ) " );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE apaccnt "
               "SET apaccnt_vendtype_id=:apaccnt_vendtype_id,"
               "    apaccnt_vendtype=:apaccnt_vendtype,"
               "    apaccnt_ap_accnt_id=:apaccnt_ap_accnt_id,"
               "    apaccnt_prepaid_accnt_id=:apaccnt_prepaid_accnt_id,"
               "    apaccnt_discount_accnt_id=:apaccnt_discount_accnt_id "
               "WHERE (apaccnt_id=:apaccnt_id);" );

  q.bindValue(":apaccnt_id", _apaccntid);
  q.bindValue(":apaccnt_ap_accnt_id", _ap->id());
  q.bindValue(":apaccnt_prepaid_accnt_id", _prepaid->id());
  q.bindValue(":apaccnt_discount_accnt_id", _discount->id());

  if (_allVendorTypes->isChecked())
  {
    q.bindValue(":apaccnt_vendtype_id", -1);
    q.bindValue(":apaccnt_vendtype", ".*");
  }
  else if (_selectedVendorType->isChecked())
  {
    q.bindValue(":apaccnt_vendtype_id", _vendorTypes->id());
    q.bindValue(":apaccnt_vendtype", "");
  }
  else if (_vendorTypePattern->isChecked())
  {
    q.bindValue(":apaccnt_vendtype_id", -1);
    q.bindValue(":apaccnt_vendtype", _vendorType->text().trimmed());
  }

  q.exec();

  done(_apaccntid);
}

void apAccountAssignment::populate()
{
  q.prepare( "SELECT apaccnt_vendtype_id, apaccnt_vendtype,"
             "       apaccnt_ap_accnt_id, apaccnt_prepaid_accnt_id,"
             "       apaccnt_discount_accnt_id "
             "FROM apaccnt "
             "WHERE (apaccnt_id=:apaccnt_id);" );
  q.bindValue(":apaccnt_id", _apaccntid);
  q.exec();
  if (q.first())
  {
    if (q.value("apaccnt_vendtype_id").toInt() == -1)
    {
      if (q.value("apaccnt_vendtype").toString() == ".*")
        _allVendorTypes->setChecked(TRUE);
      else
      {
        _vendorTypePattern->setChecked(TRUE);
        _vendorType->setText(q.value("apaccnt_vendtype").toString());
      }
    }
    else
    {
      _selectedVendorType->setChecked(TRUE);
      _vendorTypes->setId(q.value("apaccnt_vendtype_id").toInt());
    }

    _ap->setId(q.value("apaccnt_ap_accnt_id").toInt());
    _prepaid->setId(q.value("apaccnt_prepaid_accnt_id").toInt());
    _discount->setId(q.value("apaccnt_discount_accnt_id").toInt());
  }
}


