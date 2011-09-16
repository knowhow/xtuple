/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"

#include "qsqldatabaseproto.h"

/** \ingroup scriptapi
    \brief   Script Exposure of QSqlDatabase class.

    \todo implement QSqlDatabase::addDatabase(QSqlDriver*, const QString &connectionName)
    \todo implement QSqlDatabase::registerSqlDriver(const QString &, QSqlDriverCreatorBase *)
 */

QScriptValue QSqlDatabasetoScriptValue(QScriptEngine *engine, QSqlDatabase const &item)
{
  QVariant v;
  v.setValue(item);
  return engine->newVariant(v);
}

void QSqlDatabasefromScriptValue(const QScriptValue &obj, QSqlDatabase &item)
{
  item = obj.toVariant().value<QSqlDatabase>();
}

QScriptValue QSqlDatabasePtrtoScriptValue(QScriptEngine *engine, QSqlDatabase* const &item)
{
  QVariant v;
  v.setValue(item);
  return engine->newVariant(v);
}

void QSqlDatabasePtrfromScriptValue(const QScriptValue &obj, QSqlDatabase* &item)
{
  item = obj.toVariant().value<QSqlDatabase*>();
}

static QScriptValue qsqldatabase_addDatabase(QScriptContext *context, QScriptEngine *engine)
{
  QSqlDatabase result;

  if (context->argumentCount() == 1 && context->argument(0).isString())
    result = QSqlDatabase::addDatabase(context->argument(0).toString());

  else if (context->argumentCount() == 2 &&
           context->argument(0).isString() &&
           context->argument(1).isString())
    result = QSqlDatabase::addDatabase(context->argument(0).toString(),
                                       context->argument(1).toString());

  /*
  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QSqlDriver*>(context->argument(0)))
    result = QSqlDatabase::addDatabase(qscriptvalue_cast<QSqlDriver*>(context->argument(0)));
  else if (context->argumentCount() == 2 &&
           qscriptvalue_cast<QSqlDriver*>(context->argument(0)) &&
           context->argument(1).isString())
    result = QSqlDatabase::addDatabase(qscriptvalue_cast<QSqlDriver*>(context->argument(0)),
                                       context->argument(1).toString());
   */

  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlDatabase::addDatabase()");

  QVariant v;
  v.setValue(result);
  return engine->newVariant(v);
}

static QScriptValue qsqldatabase_cloneDatabase(QScriptContext *context, QScriptEngine *engine)
{
  QSqlDatabase result;

  if (context->argumentCount() == 2 &&
      // context->argument(0) is a QSqlDatabase // TODO: how do we know this?
      context->argument(1).isString())
    result = QSqlDatabase::cloneDatabase(qscriptvalue_cast<QSqlDatabase>(context->argument(0)),
                                         context->argument(1).toString());

  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlDatabase::cloneDatabase()");

  QVariant v;
  v.setValue(result);
  return engine->newVariant(v);
}

static QScriptValue qsqldatabase_connectionNames(QScriptContext * /*context*/, QScriptEngine *engine)
{
  QStringList result;

  result = QSqlDatabase::connectionNames();

  QVariant v;
  v.setValue(result);
  return engine->newVariant(v);
}

static QScriptValue qsqldatabase_contains(QScriptContext *context, QScriptEngine * /*engine*/)
{
  bool result = false;

  if (context->argumentCount() == 1 && context->argument(0).isString())
    result = QSqlDatabase::contains(context->argument(0).toString());
  else
    result = QSqlDatabase::contains();

  return QScriptValue(result);
}

static QScriptValue qsqldatabase_database(QScriptContext *context, QScriptEngine *engine)
{
  QSqlDatabase result;

  if (context->argumentCount() == 0)
    result = QSqlDatabase::database();
  else if (context->argumentCount() == 1 && context->argument(0).isString())
    result = QSqlDatabase::database(context->argument(0).toString());
  else if (context->argumentCount() == 2 &&
           context->argument(0).isString() &&
           context->argument(1).isBool())
    result = QSqlDatabase::database(context->argument(0).toString(),
                                    context->argument(1).toBool());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlDatabase::database()");

  QVariant v;
  v.setValue(result);
  return engine->newVariant(v);
}

static QScriptValue qsqldatabase_drivers(QScriptContext * /*context*/, QScriptEngine *engine)
{
  QStringList result;

  result = QSqlDatabase::drivers();

  QVariant v;
  v.setValue(result);
  return engine->newVariant(v);
}

static QScriptValue qsqldatabase_isDriverAvailable(QScriptContext *context, QScriptEngine * /*engine*/)
{
  bool result = false;

  if (context->argumentCount() == 1 && context->argument(0).isString())
    result = QSqlDatabase::isDriverAvailable(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlDatabase::isDriverAvailable()");

  return QScriptValue(result);
}

/*
static QScriptValue qsqldatabase_registerSqlDriver(QScriptContext *context, QScriptEngine *engine)
{
  if (context->argumentCount() == 2)
    QSqlDatabase::registerSqlDriver(context->argument(0).toString(), context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlDatabase::registerSqlDriver()");

  return QScriptValue();
}
*/

static QScriptValue qsqldatabase_removeDatabase(QScriptContext *context, QScriptEngine * /*engine*/)
{
  if (context->argumentCount() == 1 && context->argument(0).isString())
    QSqlDatabase::removeDatabase(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSqlDatabase::removeDatabase()");

  return QScriptValue();
}

void setupQSqlDatabaseProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QSqlDatabasetoScriptValue,    QSqlDatabasefromScriptValue);
  qScriptRegisterMetaType(engine, QSqlDatabasePtrtoScriptValue, QSqlDatabasePtrfromScriptValue);

  QScriptValue proto = engine->newQObject(new QSqlDatabaseProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSqlDatabase*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSqlDatabase>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSqlDatabase,
                                                 proto);
  engine->globalObject().setProperty("QSqlDatabase",  constructor, CTORPROPFLAGS);

  constructor.setProperty("addDatabase",      engine->newFunction(qsqldatabase_addDatabase),      STATICPROPFLAGS);
  constructor.setProperty("cloneDatabase",    engine->newFunction(qsqldatabase_cloneDatabase),    STATICPROPFLAGS);
  constructor.setProperty("connectionNames",  engine->newFunction(qsqldatabase_connectionNames),  STATICPROPFLAGS);
  constructor.setProperty("contains",         engine->newFunction(qsqldatabase_contains),         STATICPROPFLAGS);
  constructor.setProperty("database",         engine->newFunction(qsqldatabase_database),         STATICPROPFLAGS);
  constructor.setProperty("drivers",          engine->newFunction(qsqldatabase_drivers),          STATICPROPFLAGS);
  constructor.setProperty("isDriverAvailable",engine->newFunction(qsqldatabase_isDriverAvailable),STATICPROPFLAGS);
//constructor.setProperty("registerSqlDriver",engine->newFunction(qsqldatabase_registerSqlDriver),STATICPROPFLAGS);
  constructor.setProperty("removeDatabase",   engine->newFunction(qsqldatabase_removeDatabase),   STATICPROPFLAGS);

}

QScriptValue constructQSqlDatabase(QScriptContext *context,
                                    QScriptEngine *engine)
{
  QSqlDatabase *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QSqlDatabase(qscriptvalue_cast<QSqlDatabase>(context->argument(0)));
  else
    obj = new QSqlDatabase();
  return engine->toScriptValue(obj);
}

QSqlDatabaseProto::QSqlDatabaseProto(QObject *parent)
    : QObject(parent)
{
}

void QSqlDatabaseProto::close()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->close();
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().close();
}

bool QSqlDatabaseProto::commit()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->commit();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().commit();
  return false;
}

QString QSqlDatabaseProto::connectOptions()   const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->connectOptions();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().connectOptions();
  return QString();
}

QString QSqlDatabaseProto::connectionName()   const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->connectionName();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().connectionName();
  return QString();
}

QString QSqlDatabaseProto::databaseName()     const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->databaseName();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().databaseName();
  return QString();
}

QSqlDriver *QSqlDatabaseProto::driver()           const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->driver();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().driver();
  return 0;
}

QString QSqlDatabaseProto::driverName()       const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->driverName();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().driverName();
  return QString();
}

QSqlQuery QSqlDatabaseProto::exec(const QString &query) const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->exec(query);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().exec(query);
  return QSqlQuery();
}

QString QSqlDatabaseProto::hostName()         const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->hostName();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().hostName();
  return QString();
}

bool QSqlDatabaseProto::isOpen()           const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->isOpen();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().isOpen();
  return false;
}

bool QSqlDatabaseProto::isOpenError()      const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->isOpenError();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().isOpenError();
  return false;
}

bool QSqlDatabaseProto::isValid()          const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->isValid();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().isValid();
  return false;
}

QSqlError QSqlDatabaseProto::lastError()        const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->lastError();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().lastError();
  return QSqlError();
}

bool QSqlDatabaseProto::open()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->open();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().open();
  return false;
}

bool QSqlDatabaseProto::open(const QString &user, const QString &password)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->open(user, password);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().open(user, password);
  return false;
}

QString QSqlDatabaseProto::password()         const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->password();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().password();
  return QString();
}

int QSqlDatabaseProto::port()             const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->port();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().port();
  return 0;
}

QSqlIndex QSqlDatabaseProto::primaryIndex(const QString &tablename) const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->primaryIndex(tablename);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().primaryIndex(tablename);
  return QSqlIndex();
}

QSqlRecord QSqlDatabaseProto::record(const QString &tablename)       const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->record(tablename);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().record(tablename);
  return QSqlRecord();
}

bool QSqlDatabaseProto::rollback()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->rollback();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().rollback();
  return false;
}

void QSqlDatabaseProto::setConnectOptions(const QString &options)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setConnectOptions(options);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().setConnectOptions(options);
}

void QSqlDatabaseProto::setDatabaseName(const QString &name)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setDatabaseName(name);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().setDatabaseName(name);
}

void QSqlDatabaseProto::setHostName(const QString &host)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setHostName(host);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().setHostName(host);
}

void QSqlDatabaseProto::setPassword(const QString &password)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setPassword(password);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().setPassword(password);
}

void QSqlDatabaseProto::setPort(int port)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setPort(port);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().setPort(port);
}

void QSqlDatabaseProto::setUserName(const QString &name)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setUserName(name);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QSqlDatabase>().setUserName(name);
}

QStringList QSqlDatabaseProto::tables(int type) const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->tables((QSql::TableType)type);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().tables((QSql::TableType)type);
  return QStringList();
}

bool QSqlDatabaseProto::transaction()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->transaction();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().transaction();
  return false;
}

QString QSqlDatabaseProto::userName() const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->userName();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QSqlDatabase>().userName();
  return QString();
}

QString QSqlDatabaseProto::toString() const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return QString("[QSqlDatabase(driver=%1, database=%2, "
                   "host=%3, port=%4, user=%5)]")
                .arg(item->driverName()).arg(item->databaseName())
                .arg(item->hostName()).arg(item->port()).arg(item->userName());
  return QString("[QSqlDatabase(unknown)]");
}
