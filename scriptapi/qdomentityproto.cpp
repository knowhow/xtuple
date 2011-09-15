/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomentityproto.h"

#include <QDomEntity>
#include <QString>

void setupQDomEntityProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomEntityProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomEntity*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomEntity>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomEntity,
                                                 proto);
  engine->globalObject().setProperty("QDomEntity",  constructor);
}

QScriptValue constructQDomEntity(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomEntity *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomEntity();
  return engine->toScriptValue(obj);
}

QDomEntityProto::QDomEntityProto(QObject *parent)
    : QObject(parent)
{
}

int QDomEntityProto:: nodeType()     const
{
  QDomEntity *item = qscriptvalue_cast<QDomEntity*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomEntityProto:: notationName() const
{
  QDomEntity *item = qscriptvalue_cast<QDomEntity*>(thisObject());
  if (item)
    return item->notationName();
  return QString();
}

QString QDomEntityProto:: publicId()     const
{
  QDomEntity *item = qscriptvalue_cast<QDomEntity*>(thisObject());
  if (item)
    return item->publicId();
  return QString();
}

QString QDomEntityProto:: systemId()     const
{
  QDomEntity *item = qscriptvalue_cast<QDomEntity*>(thisObject());
  if (item)
    return item->systemId();
  return QString();
}

QString QDomEntityProto::toString() const
{
  QDomEntity *item = qscriptvalue_cast<QDomEntity*>(thisObject());
  if (item)
    return QString("[QDomEntity(%1=%2)]")
                   .arg(item->nodeName()).arg(item->nodeValue());
  return QString("[QDomEntity(unknown)]");
}
