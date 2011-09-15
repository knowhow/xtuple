/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLERRORPROTO_H__
#define __QSQLERRORPROTO_H__

#include <QSqlError>
#include <QObject>
#include <QtScript>

/* initially only need access to the enum. save the following in case we need it
class QString;

Q_DECLARE_METATYPE(QSqlError*)
Q_DECLARE_METATYPE(QSqlError)
*/

void setupQSqlErrorProto(QScriptEngine *engine);

QScriptValue QSqlErrorTypetoScriptValue(QScriptEngine *engine, const enum QSqlError::ErrorType &p);

void QSqlErrorTypefromScriptValue(const QScriptValue &obj, enum QSqlError::ErrorType &p);

/* initially only need access to the enum. save the following in case we need it
QScriptValue constructQSqlError(QScriptContext *context, QScriptEngine *engine);

class QSqlErrorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlErrorProto(QObject *parent);

    QSqlError ( const QString & driverText = QString(), const QString & databaseText = QString(), ErrorType type = NoError, int number = -1 );
    QSqlError ( const QSqlError & other );
    ~QSqlError ();

    Q_INVOKABLE QString   databaseText () const;
    Q_INVOKABLE QString   driverText () const;
    Q_INVOKABLE bool      isValid () const;
    Q_INVOKABLE int       number () const;
    Q_INVOKABLE void      setDatabaseText ( const QString & databaseText );
    Q_INVOKABLE void      setDriverText ( const QString & driverText );
    Q_INVOKABLE void      setNumber ( int number );
    Q_INVOKABLE void      setType ( ErrorType type );
    Q_INVOKABLE QString   text () const;
    Q_INVOKABLE ErrorType type () const;

Public Functions

};
*/

#endif
