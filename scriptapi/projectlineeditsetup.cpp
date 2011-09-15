/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projectcluster.h"
#include <QtScript>

void setupProjectLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Undefined",    QScriptValue(engine, ProjectLineEdit::Undefined),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder",   QScriptValue(engine, ProjectLineEdit::SalesOrder),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkOrder",    QScriptValue(engine, ProjectLineEdit::WorkOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrder",QScriptValue(engine, ProjectLineEdit::PurchaseOrder),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ProjectLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
