/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "parameterlistsetup.h"
#include "qtsetup.h"

QScriptValue ParameterListtoScriptValue(QScriptEngine *engine, const ParameterList &params)
{
  QScriptValue obj = engine->newObject();
  for(int i = 0; i < params.count(); i++)
  {
    obj.setProperty(params.name(i), engine->newVariant(params.value(i)));
  }

  return obj;
}

void ParameterListfromScriptValue(const QScriptValue &obj, ParameterList &params)
{
  QScriptValueIterator it(obj);
  while (it.hasNext())
  {
    it.next();
    if(it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    if (it.value().isArray())
    {
      QList<QVariant> cpplist;
      for (int i = 0;  i < it.value().property("length").toInt32(); i++)
        cpplist.append(it.value().property(i).toVariant());
      params.append(it.name(), cpplist);
    }
    else
      params.append(it.name(), it.value().toVariant());
  }
}

void setupParameterList(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, ParameterListtoScriptValue, ParameterListfromScriptValue);
}
