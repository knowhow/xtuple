/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipmentcluster.h"
#include <QtScript>

void setupShipmentClusterLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("All",          QScriptValue(engine, ShipmentClusterLineEdit::All),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder",   QScriptValue(engine, ShipmentClusterLineEdit::SalesOrder),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrder",QScriptValue(engine, ShipmentClusterLineEdit::TransferOrder),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ShipmentClusterLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
