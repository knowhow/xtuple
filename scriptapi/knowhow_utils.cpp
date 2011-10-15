#include "knohow_utils.h"

void setupQMessageBox(QScriptEngine *engine)
{
  //qScriptRegisterMetaType(engine, XTreeWidgetItemtoScriptValue,     XTreeWidgetItemfromScriptValue);
  //qScriptRegisterMetaType(engine, XTreeWidgetItemListtoScriptValue, XTreeWidgetItemListfromScriptValue);

  QScriptValue constructor = engine->newFunction(qscript_call);
  engine->globalObject().setProperty("QMessageBox", constructor, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
