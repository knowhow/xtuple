/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "ordercluster.h"
#include <QtScript>

void setupOrderLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("AnyStatus",QScriptValue(engine, OrderLineEdit::AnyStatus),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Unposted", QScriptValue(engine, OrderLineEdit::Unposted), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Open",	 QScriptValue(engine, OrderLineEdit::Open),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Closed",	 QScriptValue(engine, OrderLineEdit::Closed),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("AnyType",	QScriptValue(engine, OrderLineEdit::AnyType),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Purchase",QScriptValue(engine, OrderLineEdit::Purchase),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Return",	QScriptValue(engine, OrderLineEdit::Return),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Sales",	QScriptValue(engine, OrderLineEdit::Sales),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Transfer",QScriptValue(engine, OrderLineEdit::Transfer),  QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("OrderLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
