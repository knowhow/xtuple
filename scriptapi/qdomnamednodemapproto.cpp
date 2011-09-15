/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomnamednodemapproto.h"

#include <QDomNode>
#include <QString>

void setupQDomNamedNodeMapProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomNamedNodeMapProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomNamedNodeMap*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomNamedNodeMap>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomNamedNodeMap,
                                                 proto);
  engine->globalObject().setProperty("QDomNamedNodeMap",  constructor);
}

QScriptValue constructQDomNamedNodeMap(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomNamedNodeMap *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomNamedNodeMap();
  return engine->toScriptValue(obj);
}

QDomNamedNodeMapProto::QDomNamedNodeMapProto(QObject *parent)
    : QObject(parent)
{
}

bool QDomNamedNodeMapProto:: contains(const QString& name) const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->contains(name);
  return false;
}

int QDomNamedNodeMapProto:: count()         const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

bool QDomNamedNodeMapProto:: isEmpty()       const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

QDomNode QDomNamedNodeMapProto:: item(int index) const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->item(index);
  return QDomNode();
}

uint QDomNamedNodeMapProto:: length()        const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

QDomNode QDomNamedNodeMapProto:: namedItem(const QString& name) const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->namedItem(name);
  return QDomNode();
}

QDomNode QDomNamedNodeMapProto:: namedItemNS(const QString& nsURI, const QString& localName) const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->namedItemNS(nsURI, localName);
  return QDomNode();
}

QDomNode QDomNamedNodeMapProto:: removeNamedItem(const QString& name)
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->removeNamedItem(name);
  return QDomNode();
}

QDomNode QDomNamedNodeMapProto:: removeNamedItemNS(const QString& nsURI, const QString& localName)
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->removeNamedItemNS(nsURI, localName);
  return QDomNode();
}

QDomNode QDomNamedNodeMapProto:: setNamedItem(const QDomNode& newNode)
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->setNamedItem(newNode);
  return QDomNode();
}

QDomNode QDomNamedNodeMapProto:: setNamedItemNS(const QDomNode& newNode)
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->setNamedItemNS(newNode);
  return QDomNode();
}

int QDomNamedNodeMapProto:: size() const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QString QDomNamedNodeMapProto::toString() const
{
  QDomNamedNodeMap *item = qscriptvalue_cast<QDomNamedNodeMap*>(thisObject());
  if (item)
    return QString("[QDomNamedNodeMap(size = %1)]").arg(item->size());
  return QString("[QDomNamedNodeMap(unknown)]");
}
