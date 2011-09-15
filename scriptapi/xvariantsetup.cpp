/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xvariantsetup.h"

static QScriptValue scriptEncode(QScriptContext *context,
                                 QScriptEngine  * /*engine*/)
{
  QString result;

  if (context->argumentCount() == 1 &&
      ! qscriptvalue_cast<QVariant>(context->argument(0)).isNull())
    result = XVariant::encode(qscriptvalue_cast<QVariant>(context->argument(0)));
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate XVariant::encode()");

  return QScriptValue(result);
}

static QScriptValue scriptDecode(QScriptContext *context,
                                 QScriptEngine  *engine)
{
  QVariant result;

  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString())
    result = XVariant::decode(context->argument(0).toString(),
                              context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate XVariant::decode()");
  return engine->newVariant(result);
}

// TODO: this should really be in a QVariant exposure
static QScriptValue scriptTypeName(QScriptContext *context,
                                   QScriptEngine  * /*engine*/)
{
  QString result;

  if (context->argumentCount() == 1 &&
      ! qscriptvalue_cast<QVariant>(context->argument(0)).isNull())
    result = (qscriptvalue_cast<QVariant>(context->argument(0))).typeName();
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate XVariant::typeName()");

  return QScriptValue(result);
}

void setupXVariant(QScriptEngine *engine)
{
  QScriptValue xvar = engine->newObject();

  engine->globalObject().setProperty("XVariant", xvar, QScriptValue::ReadOnly | QScriptValue::Undeletable);

  xvar.setProperty("encode",   engine->newFunction(scriptEncode));
  xvar.setProperty("decode",   engine->newFunction(scriptDecode));
  xvar.setProperty("typeName", engine->newFunction(scriptTypeName));
}
