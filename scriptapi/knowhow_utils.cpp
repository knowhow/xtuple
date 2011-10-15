#include "knowhow_utils.h"

/* dzon
void setupQUrlProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQUrlProto entered");

  QScriptValue urlproto = engine->newQObject(new QUrlProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUrl*>(), urlproto);

  QScriptValue urlConstructor = engine->newFunction(constructQUrl, urlproto);
  engine->globalObject().setProperty("QUrl", urlConstructor);
}
*/


void setupMsgBox(QScriptEngine *engine)
{
  //qScriptRegisterMetaType(engine, XTreeWidgetItemtoScriptValue,     XTreeWidgetItemfromScriptValue);
  //qScriptRegisterMetaType(engine, XTreeWidgetItemListtoScriptValue, XTreeWidgetItemListfromScriptValue);

  QScriptValue constructor = engine->newQObject(new Wrapper_QMessageBox(engine));
  //engine->setDefaultPrototype(qMetaTypeId<Wrapper_QMessageBox*>(), constructor);
  engine->globalObject().setProperty("MsgBox", constructor, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

void Wrapper_QMessageBox::setWindowTitle ( const QString & title )
{
   QMessageBox::setWindowTitle(title);
}



