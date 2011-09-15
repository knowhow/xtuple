/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customCommandArgument.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

customCommandArgument::customCommandArgument(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));

  _mode = cNew;
  _cmdargid = -1;
  _cmdid = -1;
}

customCommandArgument::~customCommandArgument()
{
  // no need to delete child widgets, Qt does it all for us
}

void customCommandArgument::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customCommandArgument::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("cmd_id", &valid);
  if(valid)
    _cmdid = param.toInt();
  
  param = pParams.value("cmdarg_id", &valid);
  if(valid)
  {
    _cmdargid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      _mode = cNew;
    else if("edit" == mode)
      _mode = cEdit;
  }
  
  return NoError;
}

void customCommandArgument::sSave()
{
  if(_argument->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("No Argument Specified"),
                      tr("You must specify an argument in order to save.") );
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO cmdarg"
              "      (cmdarg_cmd_id, cmdarg_order, cmdarg_arg) "
              "VALUES(:cmd_id, :order, :argument);");
  else if(cEdit == _mode)
    q.prepare("UPDATE cmdarg"
              "   SET cmdarg_order=:order,"
              "       cmdarg_arg=:argument"
              " WHERE (cmdarg_id=:cmdarg_id); ");

  q.bindValue(":cmd_id", _cmdid);
  q.bindValue(":cmdarg_id", _cmdargid);
  q.bindValue(":order", _order->value());
  q.bindValue(":argument", _argument->text());

  if(q.exec())
    accept();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customCommandArgument::populate()
{
  q.prepare("SELECT cmdarg_cmd_id, cmdarg_order, cmdarg_arg"
            "  FROM cmdarg"
            " WHERE (cmdarg_id=:cmdarg_id);");
  q.bindValue(":cmdarg_id", _cmdargid);
  q.exec();
  if(q.first())
  {
    _cmdid = q.value("cmdarg_cmd_id").toInt();
    _order->setValue(q.value("cmdarg_order").toInt());
    _argument->setText(q.value("cmdarg_arg").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
