/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomnotationproto.h"

#include <QDomNotation>
#include <QString>

void setupQDomNotationProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomNotationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomNotation*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomNotation>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomNotation,
                                                 proto);
  engine->globalObject().setProperty("QDomNotation",  constructor);
}

QScriptValue constructQDomNotation(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomNotation *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomNotation();
  return engine->toScriptValue(obj);
}

QDomNotationProto::QDomNotationProto(QObject *parent)
    : QObject(parent)
{
}

int QDomNotationProto:: nodeType() const
{
  QDomNotation *item = qscriptvalue_cast<QDomNotation*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomNotationProto:: publicId() const
{
  QDomNotation *item = qscriptvalue_cast<QDomNotation*>(thisObject());
  if (item)
    return item->publicId();
  return QString();
}

QString QDomNotationProto:: systemId() const
{
  QDomNotation *item = qscriptvalue_cast<QDomNotation*>(thisObject());
  if (item)
    return item->systemId();
  return QString();
}

QString QDomNotationProto::toString() const
{
  QDomNotation *item = qscriptvalue_cast<QDomNotation*>(thisObject());
  if (item)
    return QString("[QDomNotation(system id = %1, public id = %2)]")
                .arg(item->systemId()).arg(item->publicId());
  return QString("[QDomNotation(unknown)]");
}
