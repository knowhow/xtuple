/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomcdatasectionproto.h"

#include <QDomCDATASection>

void setupQDomCDATASectionProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomCDATASectionProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomCDATASection*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomCDATASection>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomCDATASection,
                                                 proto);
  engine->globalObject().setProperty("QDomCDATASection",  constructor);
}

QScriptValue constructQDomCDATASection(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomCDATASection *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomCDATASection();
  return engine->toScriptValue(obj);
}

QDomCDATASectionProto::QDomCDATASectionProto(QObject *parent)
    : QObject(parent)
{
}

int QDomCDATASectionProto:: nodeType() const
{
  QDomCDATASection *item = qscriptvalue_cast<QDomCDATASection*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomCDATASectionProto::toString() const
{
  QDomCDATASection *item = qscriptvalue_cast<QDomCDATASection*>(thisObject());
  if (item)
    return QString("[QDomCDATASection(length=%1]").arg(item->length());
  return QString("[QDomCDATASection(unknown)]");
}
