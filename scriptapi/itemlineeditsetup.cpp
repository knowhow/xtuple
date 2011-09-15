/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemcluster.h"
#include <QtScript>

Q_DECLARE_METATYPE(ItemLineEdit*)

QScriptValue ItemLineEditToScriptValue(QScriptEngine *engine, ItemLineEdit* const &in)
{
  return engine->newQObject(in);
}

void ItemLineEditFromScriptValue(const QScriptValue &object, ItemLineEdit* &out)
{
  out = qobject_cast<ItemLineEdit*>(object.toQObject());
}


void setupItemLineEdit(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, ItemLineEditToScriptValue, ItemLineEditFromScriptValue);

  QScriptValue widget = engine->newObject();

  widget.setProperty("cUndefined",	    QScriptValue(engine, ItemLineEdit::cUndefined),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPurchased",	    QScriptValue(engine, ItemLineEdit::cPurchased),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cManufactured",	    QScriptValue(engine, ItemLineEdit::cManufactured),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPhantom",	    QScriptValue(engine, ItemLineEdit::cPhantom),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cBreeder",	    QScriptValue(engine, ItemLineEdit::cBreeder),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cCoProduct",	    QScriptValue(engine, ItemLineEdit::cCoProduct),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cByProduct",	    QScriptValue(engine, ItemLineEdit::cByProduct),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cReference",	    QScriptValue(engine, ItemLineEdit::cReference),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cCosting",	    QScriptValue(engine, ItemLineEdit::cCosting),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cTooling",	    QScriptValue(engine, ItemLineEdit::cTooling),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cOutsideProcess",	    QScriptValue(engine, ItemLineEdit::cOutsideProcess),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanning",	    QScriptValue(engine, ItemLineEdit::cPlanning),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cKit",	            QScriptValue(engine, ItemLineEdit::cKit),	             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cAllItemTypes_Mask",  QScriptValue(engine, ItemLineEdit::cAllItemTypes_Mask),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningMRP",	    QScriptValue(engine, ItemLineEdit::cPlanningMRP),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningMPS",	    QScriptValue(engine, ItemLineEdit::cPlanningMPS),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningNone",	    QScriptValue(engine, ItemLineEdit::cPlanningNone),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningAny",	    QScriptValue(engine, ItemLineEdit::cPlanningAny),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cItemActive",	    QScriptValue(engine, ItemLineEdit::cItemActive),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cSold",	            QScriptValue(engine, ItemLineEdit::cSold),	             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cLocationControlled", QScriptValue(engine, ItemLineEdit::cLocationControlled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cLotSerialControlled",QScriptValue(engine, ItemLineEdit::cLotSerialControlled),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cDefaultLocation",    QScriptValue(engine, ItemLineEdit::cDefaultLocation),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cActive",	            QScriptValue(engine, ItemLineEdit::cActive),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralManufactured",QScriptValue(engine, ItemLineEdit::cGeneralManufactured),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralPurchased",   QScriptValue(engine, ItemLineEdit::cGeneralPurchased),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralComponents",  QScriptValue(engine, ItemLineEdit::cGeneralComponents),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralInventory",   QScriptValue(engine, ItemLineEdit::cGeneralInventory),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cKitComponents",	    QScriptValue(engine, ItemLineEdit::cKitComponents),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ItemLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
