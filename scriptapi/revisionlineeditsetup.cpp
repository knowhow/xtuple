/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "revisioncluster.h"
#include <QtScript>

void setupRevisionLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("View",    QScriptValue(engine, RevisionLineEdit::View),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Use",     QScriptValue(engine, RevisionLineEdit::Use),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Maintain",QScriptValue(engine, RevisionLineEdit::Maintain),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("All", QScriptValue(engine, RevisionLineEdit::All), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOM", QScriptValue(engine, RevisionLineEdit::BOM), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOO", QScriptValue(engine, RevisionLineEdit::BOO), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("RevisionLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
