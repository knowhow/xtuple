/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCode.h"

#include <QVariant>
#include <QMessageBox>

classCode::classCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_classCode, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

classCode::~classCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void classCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse classCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _classcodeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _classCode->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _classCode->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void classCode::sSave()
{
  if (_classCode->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Class Code Entered"),
                              tr("You must enter a valid Class Code before saving this Item Type.") );
    _classCode->setFocus();
    return;
  }

  if (_mode == cEdit)
    q.prepare( "UPDATE classcode "
               "SET classcode_code=:classcode_code, classcode_descrip=:classcode_descrip "
               "WHERE (classcode_id=:classcode_id);" );
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('classcode_classcode_id_seq') AS classcode_id");
    if (q.first())
      _classcodeid = q.value("classcode_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO classcode "
               "( classcode_id, classcode_code, classcode_descrip ) "
               "VALUES "
               "( :classcode_id, :classcode_code, :classcode_descrip );" );
  }

  q.bindValue(":classcode_id", _classcodeid);
  q.bindValue(":classcode_code", _classCode->text());
  q.bindValue(":classcode_descrip", _description->text());
  q.exec();

  done(_classcodeid);
}

void classCode::sCheck()
{
  _classCode->setText(_classCode->text().trimmed());
  if ( (_mode == cNew) && (_classCode->text().length()) )
  {
    q.prepare( "SELECT classcode_id "
               "FROM classcode "
               "WHERE (UPPER(classcode_code)=UPPER(:classcode_code));" );
    q.bindValue(":classcode_code", _classCode->text());
    q.exec();
    if (q.first())
    {
      _classcodeid = q.value("classcode_id").toInt();
      _mode = cEdit;
      populate();

      _classCode->setEnabled(FALSE);
    }
  }
}

void classCode::populate()
{
  q.prepare( "SELECT classcode_code, classcode_descrip "
             "FROM classcode "
             "WHERE (classcode_id=:classcode_id);" );
  q.bindValue(":classcode_id", _classcodeid);
  q.exec();
  if (q.first())
  {
    _classCode->setText(q.value("classcode_code"));
    _description->setText(q.value("classcode_descrip"));
  }
}

