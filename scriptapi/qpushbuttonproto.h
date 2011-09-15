/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QPUSHBUTTON_H__
#define __QPUSHBUTTON_H__

#include <QObject>
#include <QtScript>
#include <QPushButton>

Q_DECLARE_METATYPE(QPushButton*)

void setupQPushButtonProto(QScriptEngine *engine);
QScriptValue constructQPushButton(QScriptContext *context, QScriptEngine *engine);

class QPushButtonProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QPushButtonProto(QObject *parent);

};

#endif
