/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QEVENTPROTO_H__
#define __QEVENTPROTO_H__

#include <QEvent>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QEvent*)
//Q_DECLARE_METATYPE(QEvent)

void setupQEventProto(QScriptEngine *engine);
//QScriptValue constructQEvent(QScriptContext *context, QScriptEngine *engine);

class QEventProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QEventProto(QObject *parent);

    Q_INVOKABLE void accept ();
    Q_INVOKABLE void ignore ();
    Q_INVOKABLE bool isAccepted () const;
    Q_INVOKABLE void setAccepted ( bool accepted );
    Q_INVOKABLE bool spontaneous () const;
};
#endif
