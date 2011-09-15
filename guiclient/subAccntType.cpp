/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "subAccntType.h"

#include <QVariant>
#include <QMessageBox>

subAccntType::subAccntType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

subAccntType::~subAccntType()
{
  // no need to delete child widgets, Qt does it all for us
}

void subAccntType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse subAccntType::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("subaccnttype_id", &valid);
  if (valid)
  {
    _subaccnttypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _code->setEnabled(FALSE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _type->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void subAccntType::sSave()
{
  if (_mode == cEdit)
  {
    q.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE ( (subaccnttype_id<>:subaccnttype_id)"
               " AND (subaccnttype_code=:subaccnttype_code) );");
    q.bindValue(":subaccnttype_id", _subaccnttypeid);
    q.bindValue(":subaccnttype_code", _code->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Subaccount Type"),
                             tr( "A Subaccount Type with the entered code already exists."
                                 "You may not create a Subaccount Type with this code." ) );
      _code->setFocus();
      return;
    }

    q.prepare( "UPDATE subaccnttype "
               "SET subaccnttype_code=:subaccnttype_code,"
               "       subaccnttype_descrip=:subaccnttype_descrip,"
               "       subaccnttype_accnt_type=:subaccnttype_accnt_type "
               "WHERE (subaccnttype_id=:subaccnttype_id);" );
    q.bindValue(":subaccnttype_id", _subaccnttypeid);
    q.bindValue(":subaccnttype_code", _code->text());
    q.bindValue(":subaccnttype_descrip", _description->text());
    if (_type->currentIndex() == 0)
      q.bindValue(":subaccnttype_accnt_type", "A");
    else if (_type->currentIndex() == 1)
      q.bindValue(":subaccnttype_accnt_type", "L");
    else if (_type->currentIndex() == 2)
      q.bindValue(":subaccnttype_accnt_type", "E");
    else if (_type->currentIndex() == 3)
      q.bindValue(":subaccnttype_accnt_type", "R");
    else if (_type->currentIndex() == 4)
      q.bindValue(":subaccnttype_accnt_type", "Q");
    q.exec();
  }
  else if (_mode == cNew)
  {
    q.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE (subaccnttype_code=:subaccnttype_code);");
    q.bindValue(":subaccnttype_code", _code->text().trimmed());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Subaccount Type"),
                             tr( "A Subaccount Type with the entered code already exists.\n"
                                 "You may not create a Subaccount Type with this code." ) );
      _code->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('subaccnttype_subaccnttype_id_seq') AS subaccnttype_id;");
    if (q.first())
      _subaccnttypeid = q.value("subaccnttype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO subaccnttype "
               "( subaccnttype_id, subaccnttype_code,"
               "  subaccnttype_descrip, subaccnttype_accnt_type ) "
               "VALUES "
               "( :subaccnttype_id, :subaccnttype_code, :subaccnttype_descrip, :subaccnttype_accnt_type );" );
    q.bindValue(":subaccnttype_id", _subaccnttypeid);
    q.bindValue(":subaccnttype_code", _code->text());
    q.bindValue(":subaccnttype_descrip", _description->text());
    if (_type->currentIndex() == 0)
      q.bindValue(":subaccnttype_accnt_type", "A");
    else if (_type->currentIndex() == 1)
      q.bindValue(":subaccnttype_accnt_type", "L");
    else if (_type->currentIndex() == 2)
      q.bindValue(":subaccnttype_accnt_type", "E");
    else if (_type->currentIndex() == 3)
      q.bindValue(":subaccnttype_accnt_type", "R");
    else if (_type->currentIndex() == 4)
      q.bindValue(":subaccnttype_accnt_type", "Q");
    q.exec();
  }

  done(_subaccnttypeid);
}

void subAccntType::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    q.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE (subaccnttype_code=:subaccnttype_code);" );
    q.bindValue(":subaccnttype_code", _code->text());
    q.exec();
    if (q.first())
    {
      _subaccnttypeid = q.value("subaccnttype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void subAccntType::populate()
{
  q.prepare( "SELECT subaccnttype_code, subaccnttype_accnt_type, subaccnttype_descrip "
             "FROM subaccnttype "
             "WHERE (subaccnttype_id=:subaccnttype_id);" );
  q.bindValue(":subaccnttype_id", _subaccnttypeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("subaccnttype_code").toString());
    _description->setText(q.value("subaccnttype_descrip").toString());
    
    if (q.value("subaccnttype_accnt_type").toString() == "A")
      _type->setCurrentIndex(0);
    else if (q.value("subaccnttype_accnt_type").toString() == "L")
      _type->setCurrentIndex(1);
    else if (q.value("subaccnttype_accnt_type").toString() == "E")
      _type->setCurrentIndex(2);
    else if (q.value("subaccnttype_accnt_type").toString() == "R")
      _type->setCurrentIndex(3);
    else if (q.value("subaccnttype_accnt_type").toString() == "Q")
      _type->setCurrentIndex(4);
  }
}

