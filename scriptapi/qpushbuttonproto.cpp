/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qpushbuttonproto.h"

#include <QScriptEngine>
#include <QPushButton>

QScriptValue QPushButtontoScriptValue(QScriptEngine *engine, QPushButton* const &item)
{
  return engine->newQObject(item);
}

void QPushButtonfromScriptValue(const QScriptValue &obj, QPushButton* &item)
{
  item = qobject_cast<QPushButton*>(obj.toQObject());
}

void setupQPushButtonProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QPushButtontoScriptValue, QPushButtonfromScriptValue);

  QScriptValue proto = engine->newQObject(new QPushButtonProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QPushButton*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQPushButton,
                                                 proto);

  engine->globalObject().setProperty("QPushButton",  constructor);
}

QScriptValue constructQPushButton(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QPushButton *obj = 0;
  if (context->argumentCount() == 2)
  {
    obj = new QPushButton(context->argument(0).toString(),
                          qobject_cast<QWidget*>(context->argument(1).toQObject()));
  }
  else if (context->argumentCount() == 1)
  {
    obj = new QPushButton(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  }
  else
    obj = new QPushButton();
  return engine->toScriptValue(obj);
}

QPushButtonProto::QPushButtonProto(QObject *parent)
    : QObject(parent)
{
}


