/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtimerproto.h"

QScriptValue QTimertoScriptValue(QScriptEngine *engine, QTimer* const &item)
{
  return engine->newQObject(item);
}

void QTimerfromScriptValue(const QScriptValue &obj, QTimer* &item)
{
  item = qobject_cast<QTimer*>(obj.toQObject());
}

void setupQTimerProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QTimertoScriptValue, QTimerfromScriptValue);

  QScriptValue proto = engine->newQObject(new QTimerProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTimer*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QTimer>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQTimer,
                                                 proto);
  engine->globalObject().setProperty("QTimer", constructor);
}

QScriptValue constructQTimer(QScriptContext *context,
                              QScriptEngine  *engine)
{
  QTimer *obj = 0;

  if (context->argumentCount() == 1 && context->argument(0).isQObject())
    obj = new QTimer(context->argument(0).toQObject());
  else
    obj = new QTimer();

  return engine->toScriptValue(obj);
}

QTimerProto::QTimerProto(QObject *parent)
    : QObject(parent)
{
}

int QTimerProto::interval() const
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    return item->interval();
  return -1;
}

void QTimerProto::setInterval(int msec)
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    item->setInterval(msec);
}

void QTimerProto::setSingleShot(bool single)
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    item->setSingleShot(single);
}

bool QTimerProto::isActive() const
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    return item->isActive();
  return false;
}

bool QTimerProto::isSingleShot() const
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    return item->isSingleShot();
  return false;
}

int QTimerProto::timerId() const
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    return item->timerId();
  return -1;
}

void QTimerProto::start(int msec)
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    item->start(msec);
}

void QTimerProto::start()
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    item->start();
}

void QTimerProto::stop()
{
  QTimer *item = qscriptvalue_cast<QTimer*>(thisObject());
  if (item)
    item->stop();
}

