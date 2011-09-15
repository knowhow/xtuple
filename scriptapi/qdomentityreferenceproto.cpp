/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomentityreferenceproto.h"

#include <QDomEntityReference>

void setupQDomEntityReferenceProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomEntityReferenceProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomEntityReference*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomEntityReference>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomEntityReference,
                                                 proto);
  engine->globalObject().setProperty("QDomEntityReference",  constructor);
}

QScriptValue constructQDomEntityReference(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomEntityReference *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomEntityReference();
  return engine->toScriptValue(obj);
}

QDomEntityReferenceProto::QDomEntityReferenceProto(QObject *parent)
    : QObject(parent)
{
}

int QDomEntityReferenceProto:: nodeType() const
{
  QDomEntityReference *item = qscriptvalue_cast<QDomEntityReference*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomEntityReferenceProto::toString() const
{
  QDomEntityReference *item = qscriptvalue_cast<QDomEntityReference*>(thisObject());
  if (item)
    return QString("[QDomEntityReference(%1=%2)]")
                   .arg(item->nodeName()).arg(item->nodeValue());
  return QString("[QDomEntityReference(unknown)]");
}
