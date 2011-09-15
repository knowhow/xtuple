/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qitemdelegateproto.h"

#define DEBUG false

QScriptValue QItemDelegateToScriptValue(QScriptEngine *engine, QItemDelegate* const &in)
 { return engine->newQObject(in); }

void QItemDelegateFromScriptValue(const QScriptValue &object, QItemDelegate* &out)
 { out = qobject_cast<QItemDelegate*>(object.toQObject()); }


void setupQItemDelegateProto(QScriptEngine *engine)
{
  //QScriptValue proto = engine->newQObject(new QItemDelegateProto(engine));
  //engine->setDefaultPrototype(qMetaTypeId<QItemDelegate*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QItemDelegate>(),  proto);

  qScriptRegisterMetaType(engine, QItemDelegateToScriptValue, QItemDelegateFromScriptValue);

  //QScriptValue constructor = engine->newFunction(constructQItemDelegate,
  //                                               proto);
  //engine->globalObject().setProperty("QItemDelegate", constructor);
}

QScriptValue constructQItemDelegate(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QItemDelegate *obj = 0;

  if (DEBUG)
  {
    qDebug("constructQItemDelegate() entered");
    for (int i = 0; i < context->argumentCount(); i++)
      qDebug("context->argument(%d) = %s", i,
             qPrintable(context->argument(i).toString()));
  }

  context->throwError(QScriptContext::UnknownError,
                      "QItemDelegate() constructor is not yet supported");

  return engine->toScriptValue(obj);
}

QItemDelegateProto::QItemDelegateProto(QObject *parent)
    : QObject(parent)
{
}

