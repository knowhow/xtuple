/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomattrproto.h"

#include <QDomAttr>
#include <QDomElement>
#include <QString>

void setupQDomAttrProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomAttrProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomAttr*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomAttr>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomAttr,
                                                 proto);
  engine->globalObject().setProperty("QDomAttr",  constructor);
}

QScriptValue constructQDomAttr(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomAttr *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomAttr();
  return engine->toScriptValue(obj);
}

QDomAttrProto::QDomAttrProto(QObject *parent)
    : QObject(parent)
{
}

QDomElement QDomAttrProto:: ownerElement() const
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    return item->ownerElement();
  return QDomElement();
}

QString QDomAttrProto:: name()         const
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QString QDomAttrProto:: value()        const
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    return item->value();
  return QString();
}

bool QDomAttrProto:: specified()    const
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    return item->specified();
  return false;
}

int QDomAttrProto:: nodeType()     const
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

void QDomAttrProto:: setValue(const QString &str)
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    item->setValue(str);
}

QString QDomAttrProto:: toString()     const
{
  QDomAttr *item = qscriptvalue_cast<QDomAttr*>(thisObject());
  if (item)
    return QString("[QDomAttr %1='%2']").arg(item->name()).arg(item->value());
  return QString();
}

