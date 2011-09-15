/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLDATABASEPROTO_H__
#define __QSQLDATABASEPROTO_H__

#include <QObject>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QtScript>

class QSqlDriver;

Q_DECLARE_METATYPE(QSqlDatabase*)
Q_DECLARE_METATYPE(QSqlDatabase)

void setupQSqlDatabaseProto(QScriptEngine *engine);
QScriptValue constructQSqlDatabase(QScriptContext *context, QScriptEngine *engine);

class QSqlDatabaseProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlDatabaseProto(QObject *parent);

    Q_INVOKABLE void         close();
    Q_INVOKABLE bool         commit();
    Q_INVOKABLE QString      connectOptions()   const;
    Q_INVOKABLE QString      connectionName()   const;
    Q_INVOKABLE QString      databaseName()     const;
    Q_INVOKABLE QSqlDriver  *driver()           const;
    Q_INVOKABLE QString      driverName()       const;
    Q_INVOKABLE QSqlQuery    exec(const QString &query = QString()) const;
    Q_INVOKABLE QString      hostName()         const;
    Q_INVOKABLE bool         isOpen()           const;
    Q_INVOKABLE bool         isOpenError()      const;
    Q_INVOKABLE bool         isValid()          const;
    Q_INVOKABLE QSqlError    lastError()        const;
    Q_INVOKABLE bool         open();
    Q_INVOKABLE bool         open(const QString &user, const QString &password);
    Q_INVOKABLE QString      password()         const;
    Q_INVOKABLE int          port()             const;
    Q_INVOKABLE QSqlIndex    primaryIndex(const QString &tablename) const;
    Q_INVOKABLE QSqlRecord   record(const QString &tablename)       const;
    Q_INVOKABLE bool         rollback();
    Q_INVOKABLE void         setConnectOptions(const QString &options = QString());
    Q_INVOKABLE void         setDatabaseName(const QString &name);
    Q_INVOKABLE void         setHostName(const QString &host);
    Q_INVOKABLE void         setPassword(const QString &password);
    Q_INVOKABLE void         setPort(int port);
    Q_INVOKABLE void         setUserName(const QString &name);
    Q_INVOKABLE QStringList  tables(int type = QSql::Tables) const;
    Q_INVOKABLE QString      toString()                      const;
    Q_INVOKABLE bool         transaction();
    Q_INVOKABLE QString      userName()                      const;
};

#endif
