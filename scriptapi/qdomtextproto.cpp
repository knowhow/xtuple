/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomtextproto.h"

#include <QDomText>

void setupQDomTextProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomTextProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomText*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomText>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomText,
                                                 proto);
  engine->globalObject().setProperty("QDomText",  constructor);
}

QScriptValue constructQDomText(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomText *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomText();
  return engine->toScriptValue(obj);
}

QDomTextProto::QDomTextProto(QObject *parent)
    : QObject(parent)
{
}

int QDomTextProto:: nodeType() const
{
  QDomText *item = qscriptvalue_cast<QDomText*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QDomText QDomTextProto:: splitText(int offset)
{
  QDomText *item = qscriptvalue_cast<QDomText*>(thisObject());
  if (item)
    return item->splitText(offset);
  return QDomText();
}

QString QDomTextProto::toString() const
{
  QDomText *item = qscriptvalue_cast<QDomText*>(thisObject());
  if (item)
    return QString("[QDomText(%1=%2)]")
                   .arg(item->nodeName()).arg(item->nodeValue());
  return QString("[QDomText(unknown)]");
}
