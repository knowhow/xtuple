/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqlrecordproto.h"

#include <QString>

void setupQSqlRecordProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSqlRecordProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSqlRecord*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSqlRecord>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSqlRecord,
                                                 proto);
  engine->globalObject().setProperty("QSqlRecord",  constructor);
}

QScriptValue constructQSqlRecord(QScriptContext *context,
                                 QScriptEngine  *engine)
{
  QSqlRecord *obj = 0;
  if (context->argumentCount() == 0)
    obj = new QSqlRecord();
  else if (context->argumentCount() == 1)
    qWarning("QSqlRecordProto copy constructor not implemented yet");

  return engine->toScriptValue(obj);
}

QSqlRecordProto::QSqlRecordProto(QObject *parent)
    : QObject(parent)
{
}

void QSqlRecordProto::append(const QSqlField & field)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->append(field);
}

void QSqlRecordProto::clear()
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->clear();
}

void QSqlRecordProto::clearValues()
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->clearValues();
}

bool QSqlRecordProto::contains(const QString &name) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->contains(name);
  return false;
}

int QSqlRecordProto::count() const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

QSqlField QSqlRecordProto::field(int index) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->field(index);
  return QSqlField();
}

QSqlField QSqlRecordProto::field(const QString &name) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->field(name);
  return QSqlField();
}

QString QSqlRecordProto::fieldName(int index) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->fieldName(index);
  return QString();
}

int QSqlRecordProto::indexOf(const QString & name) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->indexOf(name);
  return -1;
}

void QSqlRecordProto::insert(int pos, const QSqlField &field)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->insert(pos, field);
}

bool QSqlRecordProto::isEmpty() const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QSqlRecordProto::isGenerated(const QString &name) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->isGenerated(name);
  return false;
}

bool QSqlRecordProto::isGenerated(int index) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->isGenerated(index);
  return false;
}

bool QSqlRecordProto::isNull(const QString &name) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->isNull(name);
  return false;
}

bool QSqlRecordProto::isNull(int index) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->isNull(index);
  return false;
}

void QSqlRecordProto::remove(int pos)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->remove(pos);
}

void QSqlRecordProto::replace(int pos, const QSqlField &field)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->replace(pos, field);
}

void QSqlRecordProto::setGenerated(const QString &name, bool generated)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->setGenerated(name, generated);
}

void QSqlRecordProto::setGenerated(int index, bool generated)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->setGenerated(index, generated);
}

void QSqlRecordProto::setNull(int index)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->setNull(index);
}

void QSqlRecordProto::setNull(const QString &name)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->setNull(name);
}

void QSqlRecordProto::setValue(int index, const QVariant &val)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->setValue(index, val);
}

void QSqlRecordProto::setValue(const QString &name, const QVariant &val)
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    item->setValue(name, val);
}

QVariant QSqlRecordProto::value(int index) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->value(index);
  return QVariant();
}

QVariant QSqlRecordProto::value(const QString &name) const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return item->value(name);
  return QVariant();
}

/*
QString QSqlRecordProto::toString() const
{
  QSqlRecord *item = qscriptvalue_cast<QSqlRecord*>(thisObject());
  if (item)
    return QString("QSqlRecord()");
  return QString("QSqlRecord(unknown)");
}
*/
