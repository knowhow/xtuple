/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "parametereditproto.h"

#include <QStringList>

#define DEBUG false

QScriptValue ParameterEdittoScriptValue(QScriptEngine *engine, ParameterEdit* const &item)
{
  return engine->newQObject(item);
}

void ParameterEditfromScriptValue(const QScriptValue &obj, ParameterEdit* &item)
{
  item = qobject_cast<ParameterEdit*>(obj.toQObject());
}

void setupParameterEditProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, ParameterEdittoScriptValue, ParameterEditfromScriptValue);

  QScriptValue proto = engine->newQObject(new ParameterEditProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<ParameterEdit*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<ParameterEdit>(),  proto);

  QScriptValue constructor = engine->newFunction(constructParameterEdit, proto);
  engine->globalObject().setProperty("ParameterEdit",  constructor);
}

QScriptValue constructParameterEdit(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  ParameterEdit *obj = 0;
  if (context->argumentCount() == 0)
    obj = new ParameterEdit();

  else if (context->argumentCount() == 1)
    obj = new ParameterEdit(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() == 2)
    obj = new ParameterEdit(qscriptvalue_cast<QWidget*>(context->argument(0)),
                            (Qt::WindowFlags)(context->argument(1).toInt32()));

  return engine->toScriptValue(obj);
}

ParameterEditProto::ParameterEditProto(QObject *parent)
    : QObject(parent)
{
}
