/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMATTRPROTO_H__
#define __QDOMATTRPROTO_H__

#include <QDomAttr>
#include <QDomElement>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomAttr*)
Q_DECLARE_METATYPE(QDomAttr)

void setupQDomAttrProto(QScriptEngine *engine);
QScriptValue constructQDomAttr(QScriptContext *context, QScriptEngine *engine);

class QDomAttrProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomAttrProto(QObject *parent);

    Q_INVOKABLE QDomElement ownerElement() const;
    Q_INVOKABLE QString     name()         const;
    Q_INVOKABLE QString     value()        const;
    Q_INVOKABLE bool        specified()    const;
    Q_INVOKABLE int         nodeType()     const;
    Q_INVOKABLE void        setValue(const QString&);
    Q_INVOKABLE QString     toString()     const;
};

#endif
