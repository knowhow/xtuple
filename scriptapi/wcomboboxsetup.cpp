/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "warehouseCluster.h"
#include <QtScript>

void setupWComboBox(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("All",               QScriptValue(engine, WComboBox::All),               QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllActive",         QScriptValue(engine, WComboBox::AllActive),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NonTransit",        QScriptValue(engine, WComboBox::NonTransit),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Shipping",          QScriptValue(engine, WComboBox::Shipping),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Sold",              QScriptValue(engine, WComboBox::Sold),              QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Supply",            QScriptValue(engine, WComboBox::Supply),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Transit",           QScriptValue(engine, WComboBox::Transit),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllActiveInventory",QScriptValue(engine, WComboBox::AllActiveInventory),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("WComboBox", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
