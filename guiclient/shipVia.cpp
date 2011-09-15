/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipVia.h"

#include <QVariant>
#include <QMessageBox>

shipVia::shipVia(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _shipviaid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

shipVia::~shipVia()
{
  // no need to delete child widgets, Qt does it all for us
}

void shipVia::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shipVia::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("shipvia_id", &valid);
  if (valid)
  {
    _shipviaid = param.toInt();
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

      _buttonBox->setFocus();
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

void shipVia::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().trimmed().length()))
  {
    q.prepare( "SELECT shipvia_id "
               "FROM shipvia "
               "WHERE (UPPER(shipvia_code)=UPPER(:shipvia_code));" );
    q.bindValue(":shipvia_code", _code->text());
    q.exec();
    if (q.first())
    {
      _shipviaid = q.value("shipvia_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void shipVia::sSave()
{
  if (_code->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Ship Via"),
                            tr("You must enter a valid Code.") );
      return;
  }
  
  q.prepare( "SELECT shipvia_id"
             "  FROM shipvia"
             " WHERE((shipvia_id<>:shipvia_id)"
             "   AND (UPPER(shipvia_code)=UPPER(:shipvia_code)) );" );
  q.bindValue(":shipvia_id", _shipviaid);
  q.bindValue(":shipvia_code", _code->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Ship Via"),
                           tr( "The new Ship Via information cannot be saved as the new Ship Via Code that you\n"
                               "entered conflicts with an existing Ship Via.  You must uniquely name this Ship Via\n"
                               "before you may save it." ) );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipvia_shipvia_id_seq') AS _shipvia_id;");
    if (q.first())
      _shipviaid = q.value("_shipvia_id").toInt();

    q.prepare( "INSERT INTO shipvia "
               "(shipvia_id, shipvia_code, shipvia_descrip) "
               "VALUES "
               "(:shipvia_id, :shipvia_code, :shipvia_descrip);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE shipvia "
               "SET shipvia_code=:shipvia_code, shipvia_descrip=:shipvia_descrip "
               "WHERE (shipvia_id=:shipvia_id);" );
  }

  q.bindValue(":shipvia_id", _shipviaid);
  q.bindValue(":shipvia_code", _code->text().trimmed());
  q.bindValue(":shipvia_descrip", _description->text().trimmed());
  q.exec();

  done(_shipviaid);
}

void shipVia::populate()
{
  q.prepare( "SELECT shipvia_code, shipvia_descrip "
             "FROM shipvia "
             "WHERE (shipvia_id=:shipvia_id);" );
  q.bindValue(":shipvia_id", _shipviaid);
  q.exec();
  if (q.first()) 
  {
    _code->setText(q.value("shipvia_code").toString());
    _description->setText(q.value("shipvia_descrip").toString());
  }
}
