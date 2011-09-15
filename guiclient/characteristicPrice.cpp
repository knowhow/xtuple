/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristicPrice.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

characteristicPrice::characteristicPrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_char, SIGNAL(newID(int)), this, SLOT(sCharIdChanged()));
  connect(_char, SIGNAL(newID(int)), this, SLOT(sCheck()));
  connect(_value, SIGNAL(activated(int)), this, SLOT(sCheck()));

  _char->setAllowNull(TRUE);
  
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

characteristicPrice::~characteristicPrice()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristicPrice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristicPrice::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _char->populate(QString( "SELECT DISTINCT charass_char_id, char_name "
                             "FROM charass, char "
                             "WHERE ((charass_char_id=char_id) "
                             "AND (charass_target_type='I') "
                             "AND (charass_target_id= %1)) "
                             "ORDER BY char_name; ").arg(param.toInt()));
                             
  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();
  
  param = pParams.value("ipsitem_id", &valid);
  if (valid)
    _ipsitemid = param.toInt();

  param = pParams.value("ipsitemchar_id", &valid);
  if (valid)
    _ipsitemcharid = param.toInt();
  
  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

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
      populate();
      populateItemcharInfo();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _char->setEnabled(FALSE);
      _value->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      populate();

      _close->setFocus();
    }
  }

  return NoError;
}

void characteristicPrice::sSave()
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
    q.exec("SELECT nextval('ipsitemchar_ipsitemchar_id_seq') AS result;");
    if (q.first())
      _ipsitemcharid=q.value("result").toInt();
    q.prepare( "INSERT INTO ipsitemchar "
               "( ipsitemchar_id, ipsitemchar_ipsitem_id, ipsitemchar_char_id, ipsitemchar_value, ipsitemchar_price ) "
               "VALUES "
               "( :ipsitemchar_id, :ipsitem_id, :ipsitemchar_char_id, :ipsitemchar_value, :ipsitemchar_price );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE ipsitemchar "
               "SET ipsitemchar_char_id=:ipsitemchar_char_id, ipsitemchar_value=:ipsitemchar_value, ipsitemchar_price=:ipsitemchar_price "
               "WHERE (ipsitemchar_id=:ipsitemchar_id);" );

  q.bindValue(":ipsitem_id", _ipsitemid);
  q.bindValue(":ipsitemchar_id", _ipsitemcharid);
  q.bindValue(":ipsitemchar_char_id", _char->id());
  q.bindValue(":ipsitemchar_value", _value->currentText());
  q.bindValue(":ipsitemchar_price", _price->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }

  done(_ipsitemcharid);
}

void characteristicPrice::sCheck()
{
  q.prepare( "SELECT ipsitemchar_id "
             "FROM ipsitemchar "
             "WHERE ( (ipsitemchar_char_id=:char_id)"
             " AND (ipsitemchar_value=:ipsitemchar_value) "
             " AND (ipsitemchar_ipsitem_id=:ipsitem_id));" );
  q.bindValue(":ipsitem_id", _ipsitemid);
  q.bindValue(":char_id", _char->id());
  q.bindValue(":ipsitemchar_value", _value->currentText());
  q.exec();
  if (q.first())
  {
    _ipsitemcharid = q.value("ipsitemchar_id").toInt();
    _mode = cEdit;
    populate();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  else
    _mode = cNew;
  populateItemcharInfo();
}

void characteristicPrice::populate()
{
  disconnect(_char, SIGNAL(newID(int)), this, SLOT(sCheck()));
  disconnect(_value, SIGNAL(activated(int)), this, SLOT(sCheck()));
  q.prepare( "SELECT ipsitemchar_char_id, ipsitemchar_value, ipsitemchar_price  "
             "FROM ipsitemchar "
             "WHERE (ipsitemchar_id=:ipsitemchar_id);" );
  q.bindValue(":ipsitemchar_id", _ipsitemcharid);
  q.exec();
  if (q.first())
  {
    _char->setId(q.value("ipsitemchar_char_id").toInt());
    _value->setText(q.value("ipsitemchar_value").toString());
    _price->setLocalValue(q.value("ipsitemchar_price").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  connect(_char, SIGNAL(newID(int)), this, SLOT(sCheck()));
  connect(_value, SIGNAL(activated(int)), this, SLOT(sCheck()));
}

void characteristicPrice::populateItemcharInfo()
{
  q.prepare("SELECT charass_price "
            "FROM charass "
            "WHERE ((charass_target_type='I') "
            "AND (charass_target_id=:item_id) "
            "AND (charass_char_id=:char_id) "
            "AND (charass_value=:value)) ");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":char_id", _char->id());
  q.bindValue(":value", _value->currentText());
  q.exec();
  if (q.first())
    _listPrice->setLocalValue(q.value("charass_price").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  else
    _listPrice->setLocalValue(0);
}

void characteristicPrice::sCharIdChanged()
{
  XSqlQuery charass;
  charass.prepare("SELECT charass_id, charass_value "
            "FROM charass "
            "WHERE ((charass_target_type='I') "
            "AND (charass_target_id=:item_id) "
            "AND (charass_char_id=:char_id) "
            "AND (COALESCE(charass_value,'')!='')); ");
  charass.bindValue(":item_id", _itemid);
  charass.bindValue(":char_id", _char->id());
  charass.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  _value->populate(charass);
}



