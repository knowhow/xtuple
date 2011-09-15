/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingZone.h"

#include <QVariant>
#include <QMessageBox>

shippingZone::shippingZone(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _shipzoneid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

shippingZone::~shippingZone()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingZone::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shippingZone::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("shipzone_id", &valid);
  if (valid)
  {
    _shipzoneid = param.toInt();
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
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void shippingZone::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Name Entered"),
                              tr("You must enter a valid name before saving this Shipping Zone.") );
    _name->setFocus();
    return;
  }

  q.prepare("SELECT shipzone_id"
            "  FROM shipzone"
            " WHERE((shipzone_id != :shipzone_id)"
            "   AND (shipzone_name=:shipzone_name))");
  q.bindValue(":shipzone_id", _shipzoneid);
  q.bindValue(":shipzone_name", _name->text().trimmed());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Shipping Zone"),
                          tr("You have entered a duplicate Name for this Shipping Zone. "
                             "Please select a different name before saving."));
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipzone_shipzone_id_seq') AS shipzone_id");
    if (q.first())
      _shipzoneid = q.value("shipzone_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO shipzone "
               "(shipzone_id, shipzone_name, shipzone_descrip) "
               "VALUES "
               "(:shipzone_id, :shipzone_name, :shipzone_descrip);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE shipzone "
               "SET shipzone_name=:shipzone_name, shipzone_descrip=:shipzone_descrip "
               "WHERE (shipzone_id=:shipzone_id);" );

  q.bindValue(":shipzone_id", _shipzoneid);
  q.bindValue(":shipzone_name", _name->text());
  q.bindValue(":shipzone_descrip", _description->text());
  q.exec();

  done(_shipzoneid);
}

void shippingZone::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT shipzone_id "
               "FROM shipzone "
               "WHERE (UPPER(shipzone_name)=UPPER(:shipzone_name));" );
    q.bindValue(":shipzone_name", _name->text());
    q.exec();
    if (q.first())
    {
      _shipzoneid = q.value("shipzone_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void shippingZone::populate()
{
  q.prepare( "SELECT shipzone_name, shipzone_descrip "
             "FROM shipzone "
             "WHERE (shipzone_id=:shipzone_id);" );
  q.bindValue(":shipzone_id", _shipzoneid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("shipzone_name").toString());
    _description->setText(q.value("shipzone_descrip").toString());
  }
}

