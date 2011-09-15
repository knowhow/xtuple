/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomprocessinginstructionproto.h"

#include <QDomProcessingInstruction>
#include <QString>

void setupQDomProcessingInstructionProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomProcessingInstructionProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomProcessingInstruction*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomProcessingInstruction>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomProcessingInstruction,
                                                 proto);
  engine->globalObject().setProperty("QDomProcessingInstruction",  constructor);
}

QScriptValue constructQDomProcessingInstruction(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomProcessingInstruction *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomProcessingInstruction();
  return engine->toScriptValue(obj);
}

QDomProcessingInstructionProto::QDomProcessingInstructionProto(QObject *parent)
    : QObject(parent)
{
}

QString QDomProcessingInstructionProto:: data() const
{
  QDomProcessingInstruction *item = qscriptvalue_cast<QDomProcessingInstruction*>(thisObject());
  if (item)
    return item->data();
  return QString();
}

int QDomProcessingInstructionProto:: nodeType() const
{
  QDomProcessingInstruction *item = qscriptvalue_cast<QDomProcessingInstruction*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

void QDomProcessingInstructionProto:: setData(const QString& d)
{
  QDomProcessingInstruction *item = qscriptvalue_cast<QDomProcessingInstruction*>(thisObject());
  if (item)
    item->setData(d);
}

QString QDomProcessingInstructionProto:: target() const
{
  QDomProcessingInstruction *item = qscriptvalue_cast<QDomProcessingInstruction*>(thisObject());
  if (item)
    return item->target();
  return QString();
}

QString QDomProcessingInstructionProto::toString() const
{
  QDomProcessingInstruction *item = qscriptvalue_cast<QDomProcessingInstruction*>(thisObject());
  if (item)
    return QString("[QDomProcessingInstruction(%1)]").arg(item->data());
  return QString("[QDomProcessingInstruction(unknown)]");
}
