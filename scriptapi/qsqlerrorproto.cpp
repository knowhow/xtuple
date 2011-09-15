/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqlerrorproto.h"

void setupQSqlErrorProto(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  qScriptRegisterMetaType(engine, QSqlErrorTypetoScriptValue,	QSqlErrorTypefromScriptValue);
  widget.setProperty("NoError",         QScriptValue(engine, QSqlError::NoError),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ConnectionError", QScriptValue(engine, QSqlError::ConnectionError), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("StatementError",  QScriptValue(engine, QSqlError::StatementError),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransactionError",QScriptValue(engine, QSqlError::TransactionError),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UnknownError",    QScriptValue(engine, QSqlError::UnknownError),    QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("QSqlError", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

Q_DECLARE_METATYPE(enum QSqlError::ErrorType);

QScriptValue QSqlErrorTypetoScriptValue(QScriptEngine *engine, const enum QSqlError::ErrorType &p)
{
  return QScriptValue(engine, (int)p);
}

void QSqlErrorTypefromScriptValue(const QScriptValue &obj, enum QSqlError::ErrorType &p)
{
  p = (enum QSqlError::ErrorType)obj.toInt32();
}
