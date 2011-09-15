/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "productCategory.h"

#include <QVariant>
#include <QMessageBox>

productCategory::productCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_category, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

productCategory::~productCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void productCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse productCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _prodcatid = param.toInt();
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

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void productCategory::sCheck()
{
  _category->setText(_category->text().trimmed());
  if ( (_mode == cNew) && (_category->text().length()) )
  {
    q.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE (UPPER(prodcat_code)=UPPER(:prodcat_code));" );
    q.bindValue(":prodcat_code", _category->text());
    q.exec();
    if (q.first())
    {
      _prodcatid = q.value("prodcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
  }
}

void productCategory::sSave()
{
  if (_category->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Category"),
			  tr("You must name this Category before saving it."));
    _category->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE ( (prodcat_id<>:prodcat_id)"
               " AND (prodcat_code=:prodcat_code) );");
    q.bindValue(":prodcat_id", _prodcatid);
    q.bindValue(":prodcat_code", _category->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Product Category"),
                             tr( "A Product Category with the entered code already exists."
                                 "You may not create a Product Category with this code." ) );
      _category->setFocus();
      return;
    }

    q.prepare( "UPDATE prodcat "
               "SET prodcat_code=:prodcat_code, prodcat_descrip=:prodcat_descrip "
               "WHERE (prodcat_id=:prodcat_id);" );
    q.bindValue(":prodcat_id", _prodcatid);
    q.bindValue(":prodcat_code", _category->text().toUpper());
    q.bindValue(":prodcat_descrip", _description->text());
    q.exec();
  }
  else if (_mode == cNew)
  {
    q.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE (prodcat_code=:prodcat_code);");
    q.bindValue(":prodcat_code", _category->text().trimmed());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Product Category"),
                             tr( "A Product Category with the entered code already exists.\n"
                                 "You may not create a Product Category with this code." ) );
      _category->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('prodcat_prodcat_id_seq') AS prodcat_id;");
    if (q.first())
      _prodcatid = q.value("prodcat_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO prodcat "
               "( prodcat_id, prodcat_code, prodcat_descrip ) "
               "VALUES "
               "( :prodcat_id, :prodcat_code, :prodcat_descrip );" );
    q.bindValue(":prodcat_id", _prodcatid);
    q.bindValue(":prodcat_code", _category->text().toUpper());
    q.bindValue(":prodcat_descrip", _description->text());
    q.exec();
  }

  done(_prodcatid);
}

void productCategory::populate()
{
  q.prepare( "SELECT prodcat_code, prodcat_descrip "
             "FROM prodcat "
             "WHERE (prodcat_id=:prodcat_id);" );
  q.bindValue(":prodcat_id", _prodcatid);
  q.exec();
  if (q.first())
  {
    _category->setText(q.value("prodcat_code").toString());
    _description->setText(q.value("prodcat_descrip").toString());
  }
}

