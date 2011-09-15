/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "addresscluster.h"
#include <QtScript>

void setupAddressCluster(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("CHECK",     QScriptValue(engine, AddressCluster::CHECK),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CHANGEONE", QScriptValue(engine, AddressCluster::CHANGEONE), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CHANGEALL", QScriptValue(engine, AddressCluster::CHANGEALL), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("AddressCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
