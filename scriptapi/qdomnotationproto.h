/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMNOTATIONPROTO_H__
#define __QDOMNOTATIONPROTO_H__

#include <QDomNotation>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomNotation*)
Q_DECLARE_METATYPE(QDomNotation)

void setupQDomNotationProto(QScriptEngine *engine);
QScriptValue constructQDomNotation(QScriptContext *context, QScriptEngine *engine);

class QDomNotationProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomNotationProto(QObject *parent);

    Q_INVOKABLE int     nodeType() const;
    Q_INVOKABLE QString publicId() const;
    Q_INVOKABLE QString systemId() const;
    Q_INVOKABLE QString toString() const;
};

#endif
