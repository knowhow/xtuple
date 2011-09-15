/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmacctcluster.h"
#include <QtScript>

void setupCRMAcctLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Crmacct",	       QScriptValue(engine, CRMAcctLineEdit::Crmacct),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Competitor",     QScriptValue(engine, CRMAcctLineEdit::Competitor),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Cust",	       QScriptValue(engine, CRMAcctLineEdit::Cust),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Partner",	       QScriptValue(engine, CRMAcctLineEdit::Partner),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Prospect",       QScriptValue(engine, CRMAcctLineEdit::Prospect),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Taxauth",	       QScriptValue(engine, CRMAcctLineEdit::Taxauth),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vend",	       QScriptValue(engine, CRMAcctLineEdit::Vend),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustAndProspect",QScriptValue(engine, CRMAcctLineEdit::CustAndProspect),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("CRMAcctLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
