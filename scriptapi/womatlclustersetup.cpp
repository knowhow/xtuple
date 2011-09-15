/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "wocluster.h"
#include <QtScript>

void setupWomatlCluster(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("WorkOrder",  QScriptValue(engine, WomatlCluster::WorkOrder),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WoMaterial", QScriptValue(engine, WomatlCluster::WoMaterial), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Wooper",     QScriptValue(engine, WomatlCluster::Wooper),     QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("Pull",    QScriptValue(engine, WomatlCluster::Pull),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Push",    QScriptValue(engine, WomatlCluster::Push),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Mixed",   QScriptValue(engine, WomatlCluster::Mixed), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("WomatlCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
