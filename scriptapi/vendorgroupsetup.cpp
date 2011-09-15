/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorgroup.h"
#include <QtScript>

void setupVendorGroup(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("All",          QScriptValue(engine, VendorGroup::All),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Selected",     QScriptValue(engine, VendorGroup::Selected),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SelectedType", QScriptValue(engine, VendorGroup::SelectedType),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TypePattern",  QScriptValue(engine, VendorGroup::TypePattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("VendorGroup", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
