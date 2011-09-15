/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

shippingForm::shippingForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _shipformid = -1;

  connect(_name,	SIGNAL(lostFocus()),	this, SLOT(sCheck()));
  connect(_buttonBox,	SIGNAL(accepted()),	this, SLOT(sSave()));
}

shippingForm::~shippingForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingForm::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shippingForm::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("shipform_id", &valid);
  if (valid)
  {
    _shipformid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _report->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void shippingForm::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) || (_name->text().length()))
  {
    q.prepare( "SELECT shipform_id"
               "  FROM shipform"
               " WHERE((UPPER(shipform_name)=UPPER(:shipform_name))"
               "   AND (shipform_id != :shipform_id));" );
    q.bindValue(":shipform_name", _name->text());
    q.bindValue(":shipform_id", _shipformid);
    q.exec();
    if (q.first())
    {
      _shipformid = q.value("shipform_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void shippingForm::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Format Name is Invalid"),
                          tr("You must enter a valid name for this Bill of Lading Format.") );
    _name->setFocus();
    return;
  }

  sCheck();

  if (_report->id() == -1)
  {
    QMessageBox::warning( this, tr("Report Name is Invalid"),
                          tr("You must enter a select report for this Bill of Lading Format.") );
    _report->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipform_shipform_id_seq') AS shipform_id;");
    if (q.first())
      _shipformid = q.value("shipform_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO shipform "
               "(shipform_id, shipform_name, shipform_report_name) "
               "VALUES "
               "(:shipform_id, :shipform_name, :shipform_report_name);" );

  }
  if (_mode == cEdit)
    q.prepare( "UPDATE shipform "
               "SET shipform_name=:shipform_name, shipform_report_name=:shipform_report_name "
               "WHERE (shipform_id=:shipform_id);" );

  q.bindValue(":shipform_id", _shipformid);
  q.bindValue(":shipform_name", _name->text());
  q.bindValue(":shipform_report_name", _report->code());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_shipformid);
}

void shippingForm::populate()
{
  q.prepare( "SELECT * "
       	     "FROM shipform "
	     "WHERE (shipform_id=:shipform_id);" );
  q.bindValue(":shipform_id", _shipformid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("shipform_name").toString());
    _report->setCode(q.value("shipform_report_name").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
