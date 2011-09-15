/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomdocumenttypeproto.h"

#include <QDomDocumentType>
#include <QDomNamedNodeMap>
#include <QString>

void setupQDomDocumentTypeProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomDocumentTypeProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomDocumentType*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomDocumentType>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomDocumentType,
                                                 proto);
  engine->globalObject().setProperty("QDomDocumentType",  constructor);
}

QScriptValue constructQDomDocumentType(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomDocumentType *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomDocumentType();
  return engine->toScriptValue(obj);
}

QDomDocumentTypeProto::QDomDocumentTypeProto(QObject *parent)
    : QObject(parent)
{
}

QDomNamedNodeMap QDomDocumentTypeProto:: entities()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->entities();
  return QDomNamedNodeMap();
}

QString QDomDocumentTypeProto:: internalSubset() const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->internalSubset();
  return QString();
}

QString QDomDocumentTypeProto:: name()           const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

int QDomDocumentTypeProto:: nodeType()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QDomNamedNodeMap QDomDocumentTypeProto:: notations()      const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->notations();
  return QDomNamedNodeMap();
}

QString QDomDocumentTypeProto:: publicId()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->publicId();
  return QString();
}

QString QDomDocumentTypeProto:: systemId()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->systemId();
  return QString();
}

QString QDomDocumentTypeProto::toString() const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return QString("[QDomDocumentType(type = %1, public = %2, system = %3)]")
                   .arg(item->name())
                   .arg(item->publicId())
                   .arg(item->systemId());
  return QString("[QDomDocumentType(unknown)]");
}
