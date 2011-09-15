/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorType.h"

#include <QVariant>
#include <QMessageBox>

vendorType::vendorType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

vendorType::~vendorType()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vendtype_id", &valid);
  if (valid)
  {
    _vendtypeid = param.toInt();
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
      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void vendorType::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT vendtype_id "
               "FROM vendtype "
               "WHERE (UPPER(vendtype_code)=UPPER(:venttype_code));" );
    q.bindValue(":vendtype_code", _code->text());
    q.exec();
    if (q.first())
    {
      _vendtypeid = q.value("vendtype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void vendorType::sSave()
{
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Vendor Type Code"),
                              tr("You must enter a valid Code for this Vendor Type before creating it.")  );
    _code->setFocus();
    return;
  }

  q.prepare("SELECT vendtype_id"
            "  FROM vendtype"
            " WHERE((vendtype_id != :vendtype_id)"
            "   AND (vendtype_code=:vendtype_code))");
  q.bindValue(":vendtype_id", _vendtypeid);
  q.bindValue(":vendtype_code", _code->text().trimmed());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical( this, tr("Duplicate Entry"),
      tr("The Code you have entered for this Vendor Type is already in the system.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('vendtype_vendtype_id_seq') AS _vendtype_id;");
    if (q.first())
      _vendtypeid = q.value("_vendtype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO vendtype "
               "(vendtype_id, vendtype_code, vendtype_descrip) "
               "VALUES "
               "(:vendtype_id, :vendtype_code, :vendtype_descrip);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE vendtype "
               "SET vendtype_code=:vendtype_code,"
               "    vendtype_descrip=:vendtype_descrip "
               "WHERE (vendtype_id=:vendtype_id);" );

  q.bindValue(":vendtype_id", _vendtypeid);
  q.bindValue(":vendtype_code", _code->text().trimmed());
  q.bindValue(":vendtype_descrip", _description->text().trimmed());
  q.exec();

  done(_vendtypeid);
}

void vendorType::populate()
{
  q.prepare( "SELECT vendtype_code, vendtype_descrip "
             "FROM vendtype "
             "WHERE (vendtype_id=:vendtype_id);" );
  q.bindValue(":vendtype_id", _vendtypeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("vendtype_code"));
    _description->setText(q.value("vendtype_descrip"));
  }
}
