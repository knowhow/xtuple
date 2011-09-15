/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __SCRIPTQUERY_H__
#define __SCRIPTQUERY_H__

#include <QObject>
#include <QVariant>
#include <QtScript>
#include <QSqlRecord>

#include <xsqlquery.h>

Q_DECLARE_METATYPE(XSqlQuery*)
Q_DECLARE_METATYPE(XSqlQuery)

void setupXSqlQueryProto(QScriptEngine *engine);
//QScriptValue constructXSqlQuery(QScriptContext *context, QScriptEngine *engine);

class XSqlQueryProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    XSqlQueryProto(QObject * parent = 0);

    Q_INVOKABLE QSqlRecord record() const;
    Q_INVOKABLE QString    toString() const;

    Q_INVOKABLE bool isActive();
    Q_INVOKABLE bool isValid();
    Q_INVOKABLE bool isForwardOnly();
    Q_INVOKABLE bool isSelect();

    Q_INVOKABLE int size();
    Q_INVOKABLE int numRowsAffected();

    Q_INVOKABLE QVariant value(int index);
    Q_INVOKABLE QVariant value(const QString & field);

    Q_INVOKABLE QVariantMap lastError();

    Q_INVOKABLE int findFirst(int, int);
    Q_INVOKABLE int findFirst(const QString &, int);
    Q_INVOKABLE int findFirst(const QString &, const QString &);

  public slots:
    bool first();
    bool last();
    bool next();
    bool previous();
};

#endif // __SCRIPTQUERY_H__
