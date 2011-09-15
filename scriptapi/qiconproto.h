/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QICONPROTO_H__
#define __QICONPROTO_H__

#include <QIcon>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QIcon*)
Q_DECLARE_METATYPE(QIcon)

void setupQIconProto(QScriptEngine *engine);
QScriptValue constructQIcon(QScriptContext *context, QScriptEngine *engine);

class QIconProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QIconProto(QObject *parent);

    Q_INVOKABLE bool isNull() const;
    Q_INVOKABLE void addDbImage(const QString&);
    // TODO: add in all the other functions this class supports
};

#endif
