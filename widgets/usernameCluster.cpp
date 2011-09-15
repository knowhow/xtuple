/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QtScript>

#include <parameter.h>
#include <xsqlquery.h>

#include "usernamecluster.h"
#include "format.h"

UsernameLineEdit::UsernameLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "usr", "usr_id", "usr_username", "usr_propername", 0, 0, pName)
{
  setTitles(tr("User Name"), tr("User Names"));
  setType(UsersAll);
  if (_x_preferences)
  {
    if (!_x_preferences->boolean("ClusterButtons"))
    {
      menu()->removeAction(_infoAct);
      menu()->removeAction(menu()->actions().at(2));
    }
  }
}

void UsernameLineEdit::setId(const int pId)
{
  QString sql("SELECT usr_username AS number, "
              " usr_propername AS name "
              " FROM usr"
              " WHERE ((usr_id=:id)");

  if(UsersActive == _type)
    sql += " AND (usr_active)";
  else if(UsersInactive == _type)
    sql += " AND (NOT usr_active)";

  sql += " );";

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":id", pId);
  query.exec();
  setStrikeOut(!query.size());
  if(query.first())
  {
    _id = pId;
    _valid = true;
    _username = query.value("number").toString();
    setText(_username);
    _name = query.value("name").toString();
  }
  else
  {
    _id = -1;
    _valid = false;
    _name = QString();
    _description = QString();
  }

  emit newId(_id);
  emit valid(_valid);
  emit parsed();
  _parsed = true;
}

void UsernameLineEdit::setType(enum Type pType)
{
  _type = pType;

  if(UsersActive == _type)
    _extraClause = " (usr_active)";
  else if(UsersInactive == _type)
    _extraClause = " (NOT usr_active)";
}

void UsernameLineEdit::clear()
{
  setId(-1);
  _username = "";
  setText(QString());
}

void UsernameLineEdit::setUsername(const QString & pUsername)
{
  if (!pUsername.trimmed().length())
  {
    clear();
    return;
  }

  XSqlQuery query;
  QString sql("SELECT usr_id, usr_username AS number "
                "  FROM usr"
                " WHERE ((usr_username ~* :username) ");
  if(UsersActive == _type)
    sql += " AND (usr_active)";
  else if(UsersInactive == _type)
    sql += " AND (NOT usr_active)";
  sql += ") ORDER BY usr_username LIMIT 1;";

  query.prepare(sql);
  query.bindValue(":username", QString(pUsername).prepend("^"));
  query.exec();
  if(query.first())
    setId(query.value("usr_id").toInt());
  else
  {
    _username = pUsername;
    setText(_username);
    setId(-1);
  }
}

void UsernameLineEdit::sParse()
{
  if(!_parsed)
  {
    _parsed = true;
    setUsername(text());
    if (!_valid)
      clear();
  }
}

int UsernameLineEdit::id()
{
  if(hasFocus())
    sParse();
  return _id;
}

const QString & UsernameLineEdit::username()
{
  if(hasFocus())
    sParse();
  return _username;
}

UsernameList* UsernameLineEdit::listFactory()
{
    return new UsernameList(this);
}

UsernameSearch* UsernameLineEdit::searchFactory()
{
  return new UsernameSearch(this);
}

///////////////////////////////////////

UsernameCluster::UsernameCluster(QWidget * parent, const char * name)
  : VirtualCluster(parent, name)
{
  addNumberWidget(new UsernameLineEdit(this, name));
  _name->setHidden(false);
}

void UsernameCluster::addNumberWidget(UsernameLineEdit* pNumberWidget)
{
    _number = pNumberWidget;
    if (! _number)
      return;

    _grid->addWidget(_number, 0, 1);
    setFocusProxy(pNumberWidget);

    connect(_number,	SIGNAL(newId(int)),	this,	 SIGNAL(newId(int)));
    connect(_number,	SIGNAL(parsed()), 	this, 	 SLOT(sRefresh()));
    connect(_number,	SIGNAL(valid(bool)),	this,	 SIGNAL(valid(bool)));
}

void UsernameCluster::setReadOnly(const bool pReadOnly)
{
  _number->setEnabled(!pReadOnly);
}

void UsernameCluster::setUsername(const QString & pUsername)
{
  static_cast<UsernameLineEdit* >(_number)->setUsername(pUsername);
}

//////////////////////////////////////////////////////////////////////

UsernameList::UsernameList(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualList(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName( "usernameList" );

  _listTab->setColumnCount(0);
  _listTab->addColumn(tr("User Name"), -1,  Qt::AlignLeft, true, "number" );
}

//////////////////////////////////////////////////////////////////////

UsernameSearch::UsernameSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName( "usernameSearch" );

  _listTab->setColumnCount(0);
  _listTab->addColumn(tr("User Name"), -1,  Qt::AlignLeft, true, "number" );
}

// script exposure ////////////////////////////////////////////////////////////

QScriptValue UsernameLineEdittoScriptValue(QScriptEngine *engine, UsernameLineEdit* const &item)
{
  return engine->newQObject(item);
}

void UsernameLineEditfromScriptValue(const QScriptValue &obj, UsernameLineEdit* &item)
{
  item = qobject_cast<UsernameLineEdit*>(obj.toQObject());
}

QScriptValue constructUsernameLineEdit(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  UsernameLineEdit *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate UsernameLineEdit constructor");

  return engine->toScriptValue(obj);
}

void setupUsernameLineEdit(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, UsernameLineEdittoScriptValue, UsernameLineEditfromScriptValue);

  QScriptValue widget = engine->newFunction(constructUsernameLineEdit);

  widget.setProperty("UsersAll",     QScriptValue(engine, UsernameLineEdit::UsersAll),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersActive",  QScriptValue(engine, UsernameLineEdit::UsersActive),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersInactive",QScriptValue(engine, UsernameLineEdit::UsersInactive),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("UsernameLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue UsernameClustertoScriptValue(QScriptEngine *engine, UsernameCluster* const &item)
{
  return engine->newQObject(item);
}

void UsernameClusterfromScriptValue(const QScriptValue &obj, UsernameCluster* &item)
{
  item = qobject_cast<UsernameCluster*>(obj.toQObject());
}

QScriptValue constructUsernameCluster(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  UsernameCluster *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameCluster(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameCluster(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate UsernameCluster constructor");

  return engine->toScriptValue(obj);
}

void setupUsernameCluster(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, UsernameClustertoScriptValue, UsernameClusterfromScriptValue);

  QScriptValue widget = engine->newFunction(constructUsernameCluster);

  engine->globalObject().setProperty("UsernameCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
