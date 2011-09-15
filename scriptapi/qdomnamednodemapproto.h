/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMNAMEDNODEMAPPROTO_H__
#define __QDOMNAMEDNODEMAPPROTO_H__

#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomNamedNodeMap*)
Q_DECLARE_METATYPE(QDomNamedNodeMap)

void setupQDomNamedNodeMapProto(QScriptEngine *engine);
QScriptValue constructQDomNamedNodeMap(QScriptContext *context, QScriptEngine *engine);

class QDomNamedNodeMapProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomNamedNodeMapProto(QObject *parent);

    Q_INVOKABLE bool     contains(const QString& name) const;
    Q_INVOKABLE int      count()         const;
    Q_INVOKABLE bool     isEmpty()       const;
    Q_INVOKABLE QDomNode item(int index) const;
    Q_INVOKABLE uint     length()        const;
    Q_INVOKABLE QDomNode namedItem(const QString& name) const;
    Q_INVOKABLE QDomNode namedItemNS(const QString& nsURI, const QString& localName) const;
    Q_INVOKABLE QDomNode removeNamedItem(const QString& name);
    Q_INVOKABLE QDomNode removeNamedItemNS(const QString& nsURI, const QString& localName);
    Q_INVOKABLE QDomNode setNamedItem(const QDomNode& newNode);
    Q_INVOKABLE QDomNode setNamedItemNS(const QDomNode& newNode);
    Q_INVOKABLE int      size()     const;
    Q_INVOKABLE QString  toString() const;
};

#endif
