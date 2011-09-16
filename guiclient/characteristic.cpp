/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristic.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

characteristic::characteristic(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

characteristic::~characteristic()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristic::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristic::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("char_id", &valid);
  if (valid)
  {
    _charid = param.toInt();
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
// TODO
//      _mask->setEnabled(FALSE);
//      _validator->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _useGroup->setEnabled(FALSE);
      _mask->setEnabled(FALSE);
      _validator->setEnabled(FALSE);

      _items->setEnabled(FALSE);
      _customers->setEnabled(FALSE);
      _lotSerial->setEnabled(FALSE);
      _addresses->setEnabled(FALSE);
      _crmaccounts->setEnabled(FALSE);
      _contacts->setEnabled(FALSE);
      _opportunity->setEnabled(FALSE);
      _employees->setEnabled(FALSE);
      _incidents->setEnabled(false);

      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void characteristic::sSave()
{
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Name"),
			  tr("<p>You must name this Characteristic before "
			     "saving it."));
    _name->setFocus();
    return;
  }
  if (! (_items->isChecked()       || _customers->isChecked() ||
	 _lotSerial->isChecked()   || _addresses->isChecked() ||
	 _crmaccounts->isChecked() || _contacts->isChecked()  ||
         _opportunity->isChecked() || _employees->isChecked() ||
         _incidents->isChecked()))
  {
    QMessageBox::critical(this, tr("Apply Characteristic"),
			  tr("<p>You must apply this Characteristic to at "
			     "least one type of application object."));
    _items->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('char_char_id_seq') AS char_id;");
    if (q.first())
      _charid = q.value("char_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO char "
               "( char_id, char_name, char_items, char_customers, "
               "  char_contacts, char_crmaccounts, char_addresses, "
               "  char_options, char_opportunity,"
               "  char_attributes, char_lotserial, char_employees,"
               "  char_incidents, "
               "  char_notes, char_mask, char_validator ) "
               "VALUES "
               "( :char_id, :char_name, :char_items, :char_customers, "
               "  :char_contacts, :char_crmaccounts, :char_addresses, "
               "  :char_options, :char_opportunity,"
               "  :char_attributes, :char_lotserial, :char_employees,"
               "  :char_incidents, "
               "  :char_notes, :char_mask, :char_validator );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE char "
               "SET char_name=:char_name, char_items=:char_items, "
               "    char_customers=:char_customers, "
               "    char_contacts=:char_contacts, "
               "    char_crmaccounts=:char_crmaccounts, "
               "    char_addresses=:char_addresses, "
               "    char_options=:char_options,"
               "    char_attributes=:char_attributes, "
               "    char_opportunity=:char_opportunity,"
               "    char_lotserial=:char_lotserial,"
               "    char_employees=:char_employees,"
               "    char_incidents=:char_incidents,"
               "    char_notes=:char_notes,"
               "    char_mask=:char_mask,"
               "    char_validator=:char_validator "
               "WHERE (char_id=:char_id);" );

  q.bindValue(":char_id", _charid);
  q.bindValue(":char_name", _name->text());
  q.bindValue(":char_items",       QVariant(_items->isChecked()));
  q.bindValue(":char_customers",   QVariant(_customers->isChecked()));
  q.bindValue(":char_crmaccounts", QVariant(_crmaccounts->isChecked()));
  q.bindValue(":char_contacts",	   QVariant(_contacts->isChecked()));
  q.bindValue(":char_addresses",   QVariant(_addresses->isChecked()));
  q.bindValue(":char_options",     QVariant(FALSE));
  q.bindValue(":char_attributes",  QVariant(FALSE));
  q.bindValue(":char_lotserial",   QVariant(_lotSerial->isChecked()));
  q.bindValue(":char_opportunity", QVariant(_opportunity->isChecked()));
  q.bindValue(":char_employees",   QVariant(_employees->isChecked()));
  q.bindValue(":char_incidents",   QVariant(_incidents->isChecked()));
  q.bindValue(":char_notes",       _description->toPlainText().trimmed());
  if (_mask->currentText().trimmed().size() > 0)
    q.bindValue(":char_mask",        _mask->currentText());
  if (_validator->currentText().trimmed().size() > 0)
    q.bindValue(":char_validator",   _validator->currentText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_charid);
}

void characteristic::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().trimmed().length()))
  {
    q.prepare( "SELECT char_id "
               "FROM char "
               "WHERE (UPPER(char_name)=UPPER(:char_name));" );
    q.bindValue(":char_name", _name->text());
    q.exec();
    if (q.first())
    {
      _charid = q.value("char_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void characteristic::populate()
{
  q.prepare( "SELECT * "
             "FROM char "
             "WHERE (char_id=:char_id);" );
  q.bindValue(":char_id", _charid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("char_name").toString());
    _items->setChecked(q.value("char_items").toBool());
    _customers->setChecked(q.value("char_customers").toBool());
    _contacts->setChecked(q.value("char_contacts").toBool());
    _crmaccounts->setChecked(q.value("char_crmaccounts").toBool());
    _addresses->setChecked(q.value("char_addresses").toBool());
    _lotSerial->setChecked(q.value("char_lotserial").toBool());
    _opportunity->setChecked(q.value("char_opportunity").toBool());
    _employees->setChecked(q.value("char_employees").toBool());
    _incidents->setChecked(q.value("char_incidents").toBool());
    _description->setText(q.value("char_notes").toString());
    _mask->setText(q.value("char_mask").toString());
    _validator->setText(q.value("char_validator").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

