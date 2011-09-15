/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "parametergroup.h"
#include <QtScript>

void setupParameterGroup(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("AdhocGroup",     QScriptValue(engine, ParameterGroup::AdhocGroup),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PlannerCode",    QScriptValue(engine, ParameterGroup::PlannerCode),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ProductCategory",QScriptValue(engine, ParameterGroup::ProductCategory),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ClassCode",      QScriptValue(engine, ParameterGroup::ClassCode),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemGroup",      QScriptValue(engine, ParameterGroup::ItemGroup),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CostCategory",   QScriptValue(engine, ParameterGroup::CostCategory),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomerType",   QScriptValue(engine, ParameterGroup::CustomerType),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomerGroup",  QScriptValue(engine, ParameterGroup::CustomerGroup),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CurrencyNotBase",QScriptValue(engine, ParameterGroup::CurrencyNotBase),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Currency",       QScriptValue(engine, ParameterGroup::Currency),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkCenter",     QScriptValue(engine, ParameterGroup::WorkCenter),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("User",	       QScriptValue(engine, ParameterGroup::User),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("All",	QScriptValue(engine, ParameterGroup::All),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Selected",QScriptValue(engine, ParameterGroup::Selected),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Pattern",	QScriptValue(engine, ParameterGroup::Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ParameterGroup", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
