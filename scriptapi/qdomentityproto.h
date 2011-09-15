/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMENTITYPROTO_H__
#define __QDOMENTITYPROTO_H__

#include <QDomEntity>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomEntity*)
Q_DECLARE_METATYPE(QDomEntity)

void setupQDomEntityProto(QScriptEngine *engine);
QScriptValue constructQDomEntity(QScriptContext *context, QScriptEngine *engine);

class QDomEntityProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomEntityProto(QObject *parent);

    Q_INVOKABLE int     nodeType()     const;
    Q_INVOKABLE QString notationName() const;
    Q_INVOKABLE QString publicId()     const;
    Q_INVOKABLE QString systemId()     const;
    Q_INVOKABLE QString toString()     const;
};

#endif
