/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qlayoutproto.h"

#include <QString>
#include <QWidget>
#include <QtScript>

QScriptValue QLayouttoScriptValue(QScriptEngine *engine, QLayout* const &item)
{
  return engine->newQObject(item);
}

void QLayoutfromScriptValue(const QScriptValue &obj, QLayout* &item)
{
  item = qobject_cast<QLayout*>(obj.toQObject());
}

void setupQLayoutProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QLayouttoScriptValue, QLayoutfromScriptValue);

  QScriptValue layout = engine->newObject();
  engine->globalObject().setProperty("QLayout",  layout);
  layout.setProperty("SetDefaultConstraint", QScriptValue(engine, QLayout::SetDefaultConstraint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  layout.setProperty("SetFixedSize", QScriptValue(engine, QLayout::SetFixedSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  layout.setProperty("SetMinimumSize", QScriptValue(engine, QLayout::SetMinimumSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  layout.setProperty("SetMaximumSize", QScriptValue(engine, QLayout::SetMaximumSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  layout.setProperty("SetMinAndMaxSize", QScriptValue(engine, QLayout::SetMinAndMaxSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  layout.setProperty("SetNoConstraint", QScriptValue(engine, QLayout::SetNoConstraint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
