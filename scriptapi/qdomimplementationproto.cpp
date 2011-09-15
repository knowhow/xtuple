/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomimplementationproto.h"

#include <QString>
#include <QDomDocument>
#include <QDomDocumentType>
#include <QDomImplementation>

void setupQDomImplementationProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomImplementationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomImplementation*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomImplementation>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomImplementation,
                                                 proto);
  engine->globalObject().setProperty("QDomImplementation",  constructor);
}

QScriptValue constructQDomImplementation(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomImplementation *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomImplementation();
  return engine->toScriptValue(obj);
}

QDomImplementationProto::QDomImplementationProto(QObject *parent)
    : QObject(parent)
{
}

QDomDocument QDomImplementationProto:: createDocument(const QString& nsURI, const QString& qName, const QDomDocumentType& doctype)
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    return item->createDocument(nsURI, qName, doctype);
  return QDomDocument();
}

QDomDocumentType QDomImplementationProto:: createDocumentType(const QString& qName, const QString& publicId, const QString& systemId)
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    return item->createDocumentType(qName, publicId, systemId);
  return QDomDocumentType();
}

bool QDomImplementationProto:: hasFeature(const QString& feature, const QString& version) const
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    return item->hasFeature(feature, version);
  return false;
}

int QDomImplementationProto::invalidDataPolicy()
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    return item->invalidDataPolicy();
  return 0;
}

bool QDomImplementationProto::isNull()
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

void QDomImplementationProto::setInvalidDataPolicy(int policy)
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    item->setInvalidDataPolicy((QDomImplementation::InvalidDataPolicy)policy);
}

QString QDomImplementationProto::toString() const
{
  QDomImplementation *item = qscriptvalue_cast<QDomImplementation*>(thisObject());
  if (item)
    return QString("[QDomImplementation()]");
  return QString("[QDomImplementation(unknown)]");
}
