/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristicAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QRegExpValidator>

characteristicAssignment::characteristicAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()),    this, SLOT(sSave()));
  connect(_char, SIGNAL(newID(int)),   this, SLOT(sHandleMask()));

  _char->setAllowNull(TRUE);

  _listpriceLit->hide();
  _listprice->hide();
  _listprice->setValidator(omfgThis->priceVal());

  adjustSize();
}

characteristicAssignment::~characteristicAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristicAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristicAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "I";
    handleTargetType();
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "C";
    handleTargetType();
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "CRMACCT";
    handleTargetType();
  }

  param = pParams.value("addr_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "ADDR";
    handleTargetType();
  }

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "CNTCT";
    handleTargetType();
  }

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "CT";
    handleTargetType();
  }

  param = pParams.value("ls_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "LS";
    handleTargetType();
  }

  param = pParams.value("lsreg_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "LSR";
    handleTargetType();
  }

  param = pParams.value("ophead_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "OPP";
    handleTargetType();
  }

  param = pParams.value("emp_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "EMP";
    handleTargetType();
  }

  param = pParams.value("incdt_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "INCDT";
    handleTargetType();
  }



  param = pParams.value("charass_id", &valid);
  if (valid)
  {
    _charassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _char->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _char->setEnabled(FALSE);
      _value->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("showPrices", &valid);
  if (valid)
  {
    _listpriceLit->show();
    _listprice->show();
  }

  param = pParams.value("char_id", &valid);
  if (valid)
  {
    _char->setId(param.toInt());
  }

  return NoError;
}

void characteristicAssignment::sSave()
{
  if (_char->id() == -1)
  {
    QMessageBox::information( this, tr("No Characteristic Selected"),
                              tr("You must select a Characteristic before saving this Item Characteristic.") );
    _char->setFocus();
    return;
  }
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('charass_charass_id_seq') AS charass_id;");
    if (q.first())
    {
      _charassid = q.value("charass_id").toInt();

      q.prepare( "INSERT INTO charass "
                 "( charass_id, charass_target_id, charass_target_type, charass_char_id, charass_value, charass_price, charass_default ) "
                 "VALUES "
                 "( :charass_id, :charass_target_id, :charass_target_type, :charass_char_id, :charass_value, :charass_price, :charass_default );" );
    }
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE charass "
               "SET charass_char_id=:charass_char_id, charass_value=:charass_value, "
               "charass_price=:charass_price, charass_default=:charass_default "
               "WHERE (charass_id=:charass_id);" );

  q.bindValue(":charass_id", _charassid);
  q.bindValue(":charass_target_id", _targetId);
  q.bindValue(":charass_target_type", _targetType);
  q.bindValue(":charass_char_id", _char->id());
  q.bindValue(":charass_value", _value->text());
  q.bindValue(":charass_price", _listprice->toDouble());
  q.bindValue(":charass_default", QVariant(_default->isChecked()));
  q.exec();

  done(_charassid);
}

void characteristicAssignment::sCheck()
{
  if ((_mode == cNew) || (_char->id() == -1))
  {
    q.prepare( "SELECT charass_id "
               "FROM charass "
               "WHERE ( (charass_target_type=:charass_target_id)"
               " AND (charass_target_id=:charass_target_id)"
               " AND (charass_char_id=:char_id) );" );
    q.bindValue(":charass_target_type", _targetType);
    q.bindValue(":charass_target_id", _targetId);
    q.bindValue(":char_id", _char->id());
    q.exec();
    if (q.first())
    {
      _charassid = q.value("charass_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

void characteristicAssignment::populate()
{
  q.prepare( "SELECT charass_target_id, charass_target_type, charass_char_id, charass_value, charass_default, charass_price "
                   "FROM charass "
                   "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charassid);
  q.exec();
  if (q.first())
  {
    _targetId = q.value("charass_target_id").toInt();
    _targetType = q.value("charass_target_type").toString();
    handleTargetType();

    _char->setId(q.value("charass_char_id").toInt());
    _value->setText(q.value("charass_value").toString());
    _listprice->setDouble(q.value("charass_price").toDouble());
    _default->setChecked(q.value("charass_default").toBool());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristicAssignment::handleTargetType()
{
  if((_targetType != "I") && (_targetType != "CT"))
    _default->hide();

  if(_targetType != "I")
    _listprice->hide();

  QString boolColumn;
  if ((_targetType == "C") || (_targetType == "CT"))
  {
    setWindowTitle(tr("Customer Characteristic"));
    boolColumn = "char_customers";
  }
  else if (_targetType == "I")
  {
    setWindowTitle(tr("Item Characteristic"));
    boolColumn = "char_items";
  }
  else if (_targetType == "CNTCT")
  {
    setWindowTitle(tr("Contact Characteristic"));
    boolColumn = "char_contacts";
  }
  else if (_targetType == "ADDR")
  {
    setWindowTitle(tr("Address Characteristic"));
    boolColumn = "char_addresses";
  }
  else if (_targetType == "CRMACCT")
  {
    setWindowTitle(tr("CRM Account Characteristic"));
    boolColumn = "char_crmaccounts";
  }
  else if (_targetType == "LS")
  {
    setWindowTitle(tr("Lot Serial Characteristic"));
    boolColumn = "char_lotserial";
  }
  else if (_targetType == "LSR")
  {
    setWindowTitle(tr("Lot/Serial Registration Characteristic"));
    boolColumn = "char_lotserial";
  }
  else if (_targetType == "OPP")
  {
    setWindowTitle(tr("Opportunity Characteristic"));
    boolColumn = "char_opportunity";
  }
  else if (_targetType == "EMP")
  {
    setWindowTitle(tr("Employee Characteristic"));
    boolColumn = "char_employees";
  }
  else if (_targetType == "INCDT")
  {
    setWindowTitle(tr("Incident Characteristic"));
    boolColumn = "char_incidents";
  }
  _char->populate(QString("SELECT char_id, CASE WHEN (char_notes IS NULL) THEN char_name "
                          "                     ELSE (char_name || ' - ' || firstLine(char_notes)) END "
                          "FROM char "
                          "WHERE (%1) "
                          "ORDER BY char_name; ").arg(boolColumn));
}

void characteristicAssignment::sHandleMask()
{
  XSqlQuery mask;
  mask.prepare( "SELECT COALESCE(char_mask, '') AS char_mask,"
                "       COALESCE(char_validator, '.*') AS char_validator "
                "FROM char "
                "WHERE (char_id=:char_id);" );
  mask.bindValue(":char_id", _char->id());
  mask.exec();
  if (mask.first())
  {
    _value->setInputMask(mask.value("char_mask").toString());
    QRegExp rx(mask.value("char_validator").toString());
    QValidator *validator = new QRegExpValidator(rx, this);
    _value->setValidator(validator);
  }
  else if (mask.lastError().type() != QSqlError::NoError)
  {
    systemError(this, mask.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}
