/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMENTITYREFERENCEPROTO_H__
#define __QDOMENTITYREFERENCEPROTO_H__

#include <QDomEntityReference>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QDomEntityReference*)
Q_DECLARE_METATYPE(QDomEntityReference)

void setupQDomEntityReferenceProto(QScriptEngine *engine);
QScriptValue constructQDomEntityReference(QScriptContext *context, QScriptEngine *engine);

class QDomEntityReferenceProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomEntityReferenceProto(QObject *parent);

    Q_INVOKABLE int     nodeType() const;
    Q_INVOKABLE QString toString() const;
};

#endif
