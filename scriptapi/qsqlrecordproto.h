/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLRECORDPROTO_H__
#define __QSQLRECORDPROTO_H__

#include <QObject>
#include <QSqlRecord>
#include <QSqlField>
#include <QVariant>
#include <QtScript>
#include <QScriptEngine>

class QString;

Q_DECLARE_METATYPE(QSqlRecord*)
Q_DECLARE_METATYPE(QSqlRecord)

void setupQSqlRecordProto(QScriptEngine *engine);
QScriptValue constructQSqlRecord(QScriptContext *context, QScriptEngine *engine);

class QSqlRecordProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlRecordProto(QObject *parent);

    Q_INVOKABLE void      append(const QSqlField & field);
    Q_INVOKABLE void      clear();
    Q_INVOKABLE void      clearValues();
    Q_INVOKABLE bool      contains(const QString &name)     const;
    Q_INVOKABLE int       count()                           const;
    Q_INVOKABLE QSqlField field(int index)                  const;
    Q_INVOKABLE QSqlField field(const QString &name)        const;
    Q_INVOKABLE QString   fieldName(int index)              const;
    Q_INVOKABLE int       indexOf(const QString & name)     const;
    Q_INVOKABLE void      insert(int pos, const QSqlField &field);
    Q_INVOKABLE bool      isEmpty()                         const;
    Q_INVOKABLE bool      isGenerated(const QString &name)  const;
    Q_INVOKABLE bool      isGenerated(int index)            const;
    Q_INVOKABLE bool      isNull(const QString &name)       const;
    Q_INVOKABLE bool      isNull(int index)                 const;
    Q_INVOKABLE void      remove(int pos);
    Q_INVOKABLE void      replace(int pos, const QSqlField &field);
    Q_INVOKABLE void      setGenerated(const QString &name, bool generated);
    Q_INVOKABLE void      setGenerated(int index, bool generated);
    Q_INVOKABLE void      setNull(int index);
    Q_INVOKABLE void      setNull(const QString &name);
    Q_INVOKABLE void      setValue(int index, const QVariant &val);
    Q_INVOKABLE void      setValue(const QString &name, const QVariant &val);
    Q_INVOKABLE QVariant  value(int index)                  const;
    Q_INVOKABLE QVariant  value(const QString &name)        const;

};

#endif
