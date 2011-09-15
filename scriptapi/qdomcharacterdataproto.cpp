/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomcharacterdataproto.h"

#include <QDomCharacterData>
#include <QString>

void setupQDomCharacterDataProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomCharacterDataProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomCharacterData*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomCharacterData>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomCharacterData,
                                                 proto);
  engine->globalObject().setProperty("QDomCharacterData",  constructor);
}

QScriptValue constructQDomCharacterData(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomCharacterData *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomCharacterData();
  return engine->toScriptValue(obj);
}

QDomCharacterDataProto::QDomCharacterDataProto(QObject *parent)
    : QObject(parent)
{
}

void QDomCharacterDataProto:: appendData(const QString& arg)
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    item->appendData(arg);
}

QString QDomCharacterDataProto:: data() const
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    return item->data();
  return QString();
}

void QDomCharacterDataProto:: deleteData(unsigned long offset, unsigned long count)
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    item->deleteData(offset, count);
}

void QDomCharacterDataProto:: insertData(unsigned long offset, const QString& arg)
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    item->insertData(offset, arg);
}

uint QDomCharacterDataProto:: length() const
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

int QDomCharacterDataProto:: nodeType() const
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

void QDomCharacterDataProto:: replaceData(unsigned long offset, unsigned long count, const QString& arg)
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    item->replaceData(offset, count, arg);
}

void QDomCharacterDataProto:: setData(const QString&data)
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    item->setData(data);
}

QString QDomCharacterDataProto:: substringData(unsigned long offset, unsigned long count)
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    return item->substringData(offset, count);
  return QString();
}

QString QDomCharacterDataProto::toString() const
{
  QDomCharacterData *item = qscriptvalue_cast<QDomCharacterData*>(thisObject());
  if (item)
    return QString("[QDomCharacterData(length=%1)]").arg(item->length());
  return QString("[QDomCharacterData(unknown)]");
}
