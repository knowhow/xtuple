/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTIMERPROTO_H__
#define __QTIMERPROTO_H__

#include <QTimer>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QTimer*)
//Q_DECLARE_METATYPE(QTimer)

void setupQTimerProto(QScriptEngine *engine);
QScriptValue constructQTimer(QScriptContext *context, QScriptEngine *engine);

class QTimerProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY(bool active     READ isActive)
  Q_PROPERTY(int  interval   READ interval     WRITE setInterval)
  Q_PROPERTY(bool singleShot READ isSingleShot WRITE setSingleShot)

  public:
    QTimerProto(QObject *parent);

                int  interval() const;
    Q_INVOKABLE bool isActive() const;
    Q_INVOKABLE bool isSingleShot() const;
    Q_INVOKABLE void setInterval(int);
    Q_INVOKABLE void setSingleShot(bool);
    Q_INVOKABLE int  timerId() const;       

  public slots:
    void start(int);
    void start();
    void stop();

  //signals:
  //  void timeout();
};

#endif
