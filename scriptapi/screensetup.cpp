/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "screen.h"
#include <QtScript>

void setupScreen(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("New",  QScriptValue(engine, Screen::New),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Edit", QScriptValue(engine, Screen::Edit), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("View", QScriptValue(engine, Screen::View), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("NoChanges",QScriptValue(engine, Screen::NoChanges),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Save",     QScriptValue(engine, Screen::Save),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Cancel",   QScriptValue(engine, Screen::Cancel),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Screen", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
