/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "racluster.h"
#include <QtScript>

void setupRaLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("AnyStatus",QScriptValue(engine, RaLineEdit::AnyStatus),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Open",     QScriptValue(engine, RaLineEdit::Open),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Closed",   QScriptValue(engine, RaLineEdit::Closed),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("RaLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
