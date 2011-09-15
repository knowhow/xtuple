/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glcluster.h"
#include <QtScript>

void setupGLCluster(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("cUndefined",QScriptValue(engine, GLCluster::cUndefined),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cAsset",	  QScriptValue(engine, GLCluster::cAsset),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cLiability",QScriptValue(engine, GLCluster::cLiability),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cExpense",  QScriptValue(engine, GLCluster::cExpense),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cRevenue",  QScriptValue(engine, GLCluster::cRevenue),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cEquity",	  QScriptValue(engine, GLCluster::cEquity),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("GLCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
