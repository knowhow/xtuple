/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMIMPLEMENTATIONPROTO_H__
#define __QDOMIMPLEMENTATIONPROTO_H__

#include <QDomDocument>
#include <QDomDocumentType>
#include <QDomImplementation>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomImplementation*)
Q_DECLARE_METATYPE(QDomImplementation)

void setupQDomImplementationProto(QScriptEngine *engine);
QScriptValue constructQDomImplementation(QScriptContext *context, QScriptEngine *engine);

class QDomImplementationProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomImplementationProto(QObject *parent);

    Q_INVOKABLE QDomDocument     createDocument(const QString& nsURI, const QString& qName, const QDomDocumentType& doctype);
    Q_INVOKABLE QDomDocumentType createDocumentType(const QString& qName, const QString& publicId, const QString& systemId);
    Q_INVOKABLE bool             hasFeature(const QString& feature, const QString& version) const;
    Q_INVOKABLE int              invalidDataPolicy();
    Q_INVOKABLE bool             isNull();
    Q_INVOKABLE void             setInvalidDataPolicy(int policy);
    Q_INVOKABLE QString          toString() const;
};

#endif
