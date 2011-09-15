/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xsqlqueryproto.h"

#include <QSqlError>

void setupXSqlQueryProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new XSqlQueryProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XSqlQuery*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<XSqlQuery>(),  proto);

/*
  QScriptValue constructor = engine->newFunction(constructXSqlQuery,
                                                 proto);
  engine->globalObject().setProperty("XSqlQuery",  constructor);
*/
}

/*
QScriptValue constructXSqlQuery(QScriptContext * context,
                                QScriptEngine  *engine)
{
  XSqlQuery *obj = 0;
  if (context->argumentCount() == 1)
    obj = new XSqlQuery(context->argument(1).toQObject());
  else
    obj = new XSqlQuery();
  return engine->toScriptValue(obj);
}
*/

XSqlQueryProto::XSqlQueryProto(QObject * parent)
  : QObject(parent)
{
}

QSqlRecord XSqlQueryProto::record() const
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->record();
  return QSqlRecord();
}

bool XSqlQueryProto::isActive()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->isActive();
  return false;
}

bool XSqlQueryProto::isValid()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

bool XSqlQueryProto::isForwardOnly()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->isForwardOnly();
  return true;
}

bool XSqlQueryProto::isSelect()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->isSelect();
  return true;
}

bool XSqlQueryProto::first()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->first();
  return false;
}

bool XSqlQueryProto::last()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->last();
  return true;
}

bool XSqlQueryProto::next()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->next();
  return false;
}

bool XSqlQueryProto::previous()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->previous();
  return false;
}

int XSqlQueryProto::size()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

int XSqlQueryProto::numRowsAffected()
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->numRowsAffected();
  return 0;
}

QVariant XSqlQueryProto::value(int index)
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->value(index);
  return QVariant();
}

QVariant XSqlQueryProto::value(const QString & field)
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->value(field);
  return QVariant();
}

QVariantMap XSqlQueryProto::lastError()
{
  QVariantMap m;

  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
  {
    QSqlError err = item->lastError();
    m.insert("databaseText", err.databaseText());
    m.insert("driverText", err.driverText());
    m.insert("text", err.text());
    m.insert("number", err.number());
    m.insert("type", err.type());
    m.insert("isValid", QVariant(err.isValid()));
  }

  return m;
}

QString XSqlQueryProto::toString() const
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->lastQuery();
  return QString();
}

int XSqlQueryProto::findFirst(int col, int val)
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->findFirst(col, val);
  return -1;
}

int XSqlQueryProto::findFirst(const QString & col, int val)
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->findFirst(col, val);
  return -1;
}

int XSqlQueryProto::findFirst(const QString & col, const QString & val)
{
  XSqlQuery *item = qscriptvalue_cast<XSqlQuery*>(thisObject());
  if (item)
    return item->findFirst(col, val);
  return -1;
}

