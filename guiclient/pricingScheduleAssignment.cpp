/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "pricingScheduleAssignment.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a pricingScheduleAssignment as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
pricingScheduleAssignment::pricingScheduleAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sAssign()));
  connect(_selectedCustomerShipto, SIGNAL(toggled(bool)), _customerShipto, SLOT(setEnabled(bool)));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sCustomerSelected()));

  _customerTypes->setType(XComboBox::CustomerTypes);

  _ipshead->setAllowNull(true);
  _ipshead->populate( "SELECT ipshead_id, (ipshead_name || ' - ' || ipshead_descrip) "
                      "FROM ipshead "
                      "WHERE (CURRENT_DATE < ipshead_expires) "
                      "ORDER BY ipshead_name;" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
pricingScheduleAssignment::~pricingScheduleAssignment()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void pricingScheduleAssignment::languageChange()
{
    retranslateUi(this);
}

enum SetResponse pricingScheduleAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("ipsass_id", &valid);
  if (valid)
  {
    _ipsassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _cust->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _customerGroup->setEnabled(FALSE);
      _ipshead->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void pricingScheduleAssignment::sAssign()
{
  if (!_ipshead->isValid())
  {
    QMessageBox::critical(this, tr("Cannot Save Pricing Schedule Assignment"),
                          tr("<p>You must select a Pricing Schedule."));
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT ipsass_id "
               "FROM ipsass "
               "WHERE ( (ipsass_ipshead_id=:ipsass_ipshead_id)"
               "  AND   (ipsass_cust_id=:ipsass_cust_id)"
               "  AND   (ipsass_shipto_id=:ipsass_shipto_id)"
               "  AND   (ipsass_shipto_pattern=:ipsass_shipto_pattern)"
               "  AND   (ipsass_custtype_id=:ipsass_custtype_id)"
               "  AND   (ipsass_custtype_pattern=:ipsass_custtype_pattern) );" );

    q.bindValue(":ipsass_ipshead_id", _ipshead->id());

    if (_selectedCustomer->isChecked() || _selectedShiptoPattern->isChecked())
      q.bindValue(":ipsass_cust_id", _cust->id());
    else
      q.bindValue(":ipsass_cust_id", -1);

    if (_selectedCustomerShipto->isChecked())
      q.bindValue(":ipsass_shipto_id", _customerShipto->id());
    else
      q.bindValue(":ipsass_shipto_id", -1);
  
    if (_selectedCustomerType->isChecked())
      q.bindValue(":ipsass_custtype_id", _customerTypes->id());
    else
      q.bindValue(":ipsass_custtype_id", -1);

    if (_customerTypePattern->isChecked())
      q.bindValue(":ipsass_custtype_pattern", _customerType->text());
    else
      q.bindValue(":ipsass_custtype_pattern", "");

    if (_selectedShiptoPattern->isChecked())
      q.bindValue(":ipsass_shipto_pattern", _shiptoPattern->text());
    else
      q.bindValue(":ipsass_shipto_pattern", "");

    q.exec();
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Pricing Schedule Assignment"),
                            tr("<p>This Pricing Schedule Assignment already exists."));
      return;
    }
               
    q.exec("SELECT NEXTVAL('ipsass_ipsass_id_seq') AS ipsass_id;");
    if (q.first())
      _ipsassid = q.value("ipsass_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO ipsass "
               "( ipsass_id, ipsass_ipshead_id,"
               "  ipsass_cust_id, ipsass_shipto_id, ipsass_shipto_pattern,"
               "  ipsass_custtype_id, ipsass_custtype_pattern ) "
               "VALUES "
               "( :ipsass_id, :ipsass_ipshead_id,"
               "  :ipsass_cust_id, :ipsass_shipto_id, :ipsass_shipto_pattern,"
               "  :ipsass_custtype_id, :ipsass_custtype_pattern );" );
  }
  else
    q.prepare( "UPDATE ipsass "
               "SET ipsass_id=:ipsass_id, ipsass_ipshead_id=:ipsass_ipshead_id,"
               "    ipsass_cust_id=:ipsass_cust_id,"
               "    ipsass_shipto_id=:ipsass_shipto_id,"
               "    ipsass_shipto_pattern=:ipsass_shipto_pattern,"
               "    ipsass_custtype_id=:ipsass_custtype_id,"
               "    ipsass_custtype_pattern=:ipsass_custtype_pattern "
               "WHERE (ipsass_id=:ipsass_id);" );

  q.bindValue(":ipsass_id", _ipsassid);
  q.bindValue(":ipsass_ipshead_id", _ipshead->id());

  if (_selectedCustomer->isChecked() || _selectedShiptoPattern->isChecked())
    q.bindValue(":ipsass_cust_id", _cust->id());
  else
    q.bindValue(":ipsass_cust_id", -1);

  if (_selectedCustomerShipto->isChecked())
    q.bindValue(":ipsass_shipto_id", _customerShipto->id());
  else
    q.bindValue(":ipsass_shipto_id", -1);
  
  if (_selectedCustomerType->isChecked())
    q.bindValue(":ipsass_custtype_id", _customerTypes->id());
  else
    q.bindValue(":ipsass_custtype_id", -1);

  if (_customerTypePattern->isChecked())
    q.bindValue(":ipsass_custtype_pattern", _customerType->text());
  else
    q.bindValue(":ipsass_custtype_pattern", "");

  if (_selectedShiptoPattern->isChecked())
    q.bindValue(":ipsass_shipto_pattern", _shiptoPattern->text());
  else
    q.bindValue(":ipsass_shipto_pattern", "");

  q.exec();

  done(_ipsassid);
}

void pricingScheduleAssignment::populate()
{
  q.prepare( "SELECT ipsass_ipshead_id, ipsass_cust_id,"
             "       ipsass_custtype_id, ipsass_custtype_pattern,"
             "       ipsass_shipto_pattern,"
             "       ipsass_shipto_id, shipto_cust_id "
             "FROM ipsass LEFT OUTER JOIN shipto ON (ipsass_shipto_id=shipto_id) "
             "WHERE (ipsass_id=:ipsass_id);" );
  q.bindValue(":ipsass_id", _ipsassid);
  q.exec();
  if (q.first())
  {
    _ipshead->setId(q.value("ipsass_ipshead_id").toInt());

    if (!q.value("ipsass_shipto_pattern").toString().isEmpty())
    {
      _selectedShiptoPattern->setChecked(TRUE);
      _shiptoPattern->setText(q.value("ipsass_shipto_pattern").toString());
      _cust->setId(q.value("ipsass_cust_id").toInt());
    }
    else if (q.value("ipsass_cust_id").toInt() != -1)
    {
      _selectedCustomer->setChecked(TRUE);
      _cust->setId(q.value("ipsass_cust_id").toInt());
    }
    else if (q.value("ipsass_shipto_id").toInt() != -1)
    {
      int shiptoid = q.value("ipsass_shipto_id").toInt();
      _selectedCustomerShipto->setChecked(TRUE);
      _cust->setId(q.value("shipto_cust_id").toInt());
      _customerShipto->setId(shiptoid);
    }
    else if (q.value("ipsass_custtype_id").toInt() != -1)
    {
      _selectedCustomerType->setChecked(TRUE);
      _customerTypes->setId(q.value("ipsass_custtype_id").toInt());
    }
    else
    {
      _customerTypePattern->setChecked(TRUE);
      _customerType->setText(q.value("ipsass_custtype_pattern").toString());
    }
  }
}

void pricingScheduleAssignment::sCustomerSelected()
{
  _customerShipto->clear();
  q.prepare("SELECT shipto_id, shipto_num"
            "  FROM shipto"
            " WHERE (shipto_cust_id=:cust_id); ");
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _customerShipto->populate(q);
}

