/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "warehouseZone.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

warehouseZone::warehouseZone(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

warehouseZone::~warehouseZone()
{
  // no need to delete child widgets, Qt does it all for us
}

void warehouseZone::languageChange()
{
  retranslateUi(this);
}

enum SetResponse warehouseZone::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehousid = param.toInt();

  param = pParams.value("whsezone_id", &valid);
  if (valid)
  {
    _whsezoneid = param.toInt();
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
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void warehouseZone::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Name Entered"),
                              tr("<p>You must enter a valid name before saving "
                                 "this Site Zone.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare("SELECT NEXTVAL('whsezone_whsezone_id_seq') AS whsezone_id");
    q.exec();
    if (q.first())
      _whsezoneid = q.value("whsezone_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO whsezone "
               "(whsezone_id, whsezone_warehous_id, whsezone_name, whsezone_descrip) "
               "VALUES "
               "(:whsezone_id, :warehous_id, :whsezone_name, :whsezone_descrip);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE whsezone "
               "SET whsezone_warehous_id=:warehous_id,"
               "    whsezone_name=:whsezone_name, whsezone_descrip=:whsezone_descrip "
               "WHERE (whsezone_id=:whsezone_id);" );

  q.bindValue(":whsezone_id", _whsezoneid);
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":whsezone_name", _name->text());
  q.bindValue(":whsezone_descrip", _description->text());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_whsezoneid);
}

void warehouseZone::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT whsezone_id "
               "FROM whsezone "
               "WHERE ( (whsezone_warehous_id=:warehous_id)"
               " AND (UPPER(whsezone_name)=UPPER(:whsezone_name)) );" );
    q.bindValue(":warehous_id", _warehousid);
    q.bindValue(":whsezone_name", _name->text());
    q.exec();
    if (q.first())
    {
      _whsezoneid = q.value("whsezone_id").toInt();
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

void warehouseZone::populate()
{
  q.prepare( "SELECT whsezone_warehous_id, whsezone_name, whsezone_descrip "
             "FROM whsezone "
             "WHERE (whsezone_id=:whsezone_id);" );
  q.bindValue(":whsezone_id", _whsezoneid);
  q.exec();
  if (q.first())
  {
    _warehousid = q.value("whsezone_warehous_id").toInt();
    _name->setText(q.value("whsezone_name").toString());
    _description->setText(q.value("whsezone_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
