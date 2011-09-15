/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qvalidatorproto.h"

#define DEBUG false

QScriptValue QValidatorToScriptValue(QScriptEngine *engine, QValidator* const &in)
 { return engine->newQObject(in); }

void QValidatorFromScriptValue(const QScriptValue &object, QValidator* &out)
 { out = qobject_cast<QValidator*>(object.toQObject()); }


void setupQValidatorProto(QScriptEngine *engine)
{
  //QScriptValue proto = engine->newQObject(new QValidatorProto(engine));
  //engine->setDefaultPrototype(qMetaTypeId<QValidator*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QValidator>(),  proto);

  qScriptRegisterMetaType(engine, QValidatorToScriptValue, QValidatorFromScriptValue);

  //QScriptValue constructor = engine->newFunction(constructQValidator,
  //                                               proto);
  //engine->globalObject().setProperty("QValidator", constructor);
}

QScriptValue constructQValidator(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QValidator *obj = 0;

  if (DEBUG)
  {
    qDebug("constructQValidator() entered");
    for (int i = 0; i < context->argumentCount(); i++)
      qDebug("context->argument(%d) = %s", i,
             qPrintable(context->argument(i).toString()));
  }

  context->throwError(QScriptContext::UnknownError,
                      "QValidator() constructor is not yet supported");

  return engine->toScriptValue(obj);
}

QValidatorProto::QValidatorProto(QObject *parent)
    : QObject(parent)
{
}

