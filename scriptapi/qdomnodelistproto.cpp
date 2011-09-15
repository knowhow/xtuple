/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomnodelistproto.h"

#include <QDomNode>

void setupQDomNodeListProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomNodeListProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomNodeList*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomNodeList>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomNodeList,
                                                 proto);
  engine->globalObject().setProperty("QDomNodeList",  constructor);
}

QScriptValue constructQDomNodeList(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomNodeList *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomNodeList();
  return engine->toScriptValue(obj);
}

QDomNodeListProto::QDomNodeListProto(QObject *parent)
    : QObject(parent)
{
}

QDomNode QDomNodeListProto:: at(int index)   const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return item->at(index);
  return QDomNode();
}

int QDomNodeListProto:: count()         const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

bool QDomNodeListProto:: isEmpty()       const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

QDomNode QDomNodeListProto:: item(int index) const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return item->item(index);
  return QDomNode();
}

uint QDomNodeListProto:: length()        const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

int QDomNodeListProto:: size()          const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QString QDomNodeListProto::toString() const
{
  QDomNodeList *item = qscriptvalue_cast<QDomNodeList*>(thisObject());
  if (item)
    return QString("[QDomNodeList(size=%1)]").arg(item->size());
  return QString("[QDomNodeList(unknown)]");
}
