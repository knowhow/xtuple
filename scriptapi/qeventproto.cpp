/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qeventproto.h"

#include <QString>

#define DEBUG false

void setupQEventProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QEventProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QEvent*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QEvent>(),  proto);

  //QScriptValue constructor = engine->newFunction(constructQEvent, proto);
  //engine->globalObject().setProperty("QEvent", constructor);
}

/*
QScriptValue constructQEvent(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QEvent *obj = 0;

  if (DEBUG)
  {
    qDebug("constructQEvent() entered");
    for (int i = 0; i < context->argumentCount(); i++)
      qDebug("context->argument(%d) = %s", i,
             qPrintable(context->argument(i).toString()));
  }

  if (context->argumentCount() >= 4)
    obj = new QEvent(context->argument(0).toString(),
                    context->argument(1).toInt32(),
                    context->argument(2).toInt32(),
                    context->argument(3).toBool());
  else if (context->argumentCount() == 3)
    obj = new QEvent(context->argument(0).toString(),
                    context->argument(1).toInt32(),
                    context->argument(2).toInt32());
  else if (context->argumentCount() == 2 &&
           qscriptvalue_cast<QPaintDevice*>(context->argument(1)))
    obj = new QEvent(qscriptvalue_cast<QEvent>(context->argument(0)),
                    qscriptvalue_cast<QPaintDevice*>(context->argument(1)));
  else if (context->argumentCount() == 2)
    obj = new QEvent(context->argument(0).toString(),
                    context->argument(1).toInt32());
  else if (context->argumentCount() == 1 &&
          context->argument(0).isString())
  {
    if (DEBUG) qDebug("calling new QEvent(QString)");
    obj = new QEvent(context->argument(0).toString());
  }
  else if (context->argumentCount() == 1)
    obj = new QEvent(qscriptvalue_cast<QEvent>(context->argument(0)));
  else
    obj = new QEvent();

  return engine->toScriptValue(obj);
}
*/

QEventProto::QEventProto(QObject *parent)
    : QObject(parent)
{
}

void QEventProto::accept()
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    item->accept();
}

void QEventProto::ignore()
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    item->ignore();
}

bool QEventProto::isAccepted() const
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    return item->isAccepted();
  return false;
}

void QEventProto::setAccepted(bool accepted)
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    item->setAccepted(accepted);
}

bool QEventProto::spontaneous() const
{
  QEvent *item = qscriptvalue_cast<QEvent*>(thisObject());
  if (item)
    return item->spontaneous();
  return false;
}

