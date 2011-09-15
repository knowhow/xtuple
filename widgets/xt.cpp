/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xt.h"

void setupXt(QScriptEngine *engine)
{
  QScriptValue glob = engine->newObject();

  glob.setProperty("RawRole",         QScriptValue(engine, Xt::RawRole),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("ScaleRole",       QScriptValue(engine, Xt::ScaleRole),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("IdRole",          QScriptValue(engine, Xt::IdRole),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("RunningSetRole",  QScriptValue(engine, Xt::RunningSetRole),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("RunningInitRole", QScriptValue(engine, Xt::RunningInitRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("TotalSetRole",    QScriptValue(engine, Xt::TotalSetRole),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("TotalInitRole",   QScriptValue(engine, Xt::TotalInitRole),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("IndentRole",      QScriptValue(engine, Xt::IndentRole),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("DeletedRole",     QScriptValue(engine, Xt::DeletedRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);

  glob.setProperty("AllModules",         QScriptValue(engine, Xt::AllModules),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("AccountingModule",   QScriptValue(engine, Xt::AccountingModule),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("SalesModule",        QScriptValue(engine, Xt::SalesModule),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("CRMModule",          QScriptValue(engine, Xt::CRMModule),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("ManufactureModule",  QScriptValue(engine, Xt::ManufactureModule),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("PurchaseModule",     QScriptValue(engine, Xt::PurchaseModule),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("ScheduleModule",     QScriptValue(engine, Xt::ScheduleModule),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("InventoryModule",    QScriptValue(engine, Xt::InventoryModule), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("ProductsModule",     QScriptValue(engine, Xt::ProductsModule),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("SystemModule",       QScriptValue(engine, Xt::SystemModule),    QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Xt", glob, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

Q_DECLARE_METATYPE(enum Xt::ItemDataRole);
Q_DECLARE_METATYPE(enum Xt::StandardModules);
