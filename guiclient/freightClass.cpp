/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "freightClass.h"

#include <QVariant>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>

freightClass::freightClass(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_freightClass, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

freightClass::~freightClass()
{
  // no need to delete child widgets, Qt does it all for us
}

void freightClass::languageChange()
{
  retranslateUi(this);
}

enum SetResponse freightClass::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("freightclass_id", &valid);
  if (valid)
  {
    _freightclassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _buttonBox->button(QDialogButtonBox::Save)->setEnabled(FALSE);
      _freightClass->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _freightClass->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void freightClass::sSave()
{
  if (_freightClass->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Freight Class Entered"),
                              tr("You must enter a valid Freight Class before saving this Item Type.") );
    _freightClass->setFocus();
    return;
  }

  if (_mode == cEdit)
    q.prepare( "UPDATE freightclass "
               "SET freightclass_code=:freightclass_code, freightclass_descrip=:freightclass_descrip "
               "WHERE (freightclass_id=:freightclass_id);" );
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('freightclass_freightclass_id_seq') AS freightclass_id");
    if (q.first())
      _freightclassid = q.value("freightclass_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO freightclass "
               "( freightclass_id, freightclass_code, freightclass_descrip ) "
               "VALUES "
               "( :freightclass_id, :freightclass_code, :freightclass_descrip );" );
  }

  q.bindValue(":freightclass_id", _freightclassid);
  q.bindValue(":freightclass_code", _freightClass->text());
  q.bindValue(":freightclass_descrip", _description->text());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_freightclassid);
}

void freightClass::sCheck()
{
  _freightClass->setText(_freightClass->text().trimmed());
  if ( (_mode == cNew) && (_freightClass->text().length()) )
  {
    q.prepare( "SELECT freightclass_id "
               "FROM freightclass "
               "WHERE (UPPER(freightclass_code)=UPPER(:freightclass_code));" );
    q.bindValue(":freightclass_code", _freightClass->text());
    q.exec();
    if (q.first())
    {
      _freightclassid = q.value("freightclass_id").toInt();
      _mode = cEdit;
      populate();

      _freightClass->setEnabled(FALSE);
    }
  }

  _buttonBox->button(QDialogButtonBox::Save)->setEnabled(TRUE);
}

void freightClass::populate()
{
  q.prepare( "SELECT * "
             "FROM freightclass "
             "WHERE (freightclass_id=:freightclass_id);" );
  q.bindValue(":freightclass_id", _freightclassid);
  q.exec();
  if (q.first())
  {
    _freightClass->setText(q.value("freightclass_code"));
    _description->setText(q.value("freightclass_descrip"));
  }
}

