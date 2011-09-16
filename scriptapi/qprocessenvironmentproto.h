/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright(c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#ifndef __QPROCESSENVIRONMENTPROTO_H__
#define __QPROCESSENVIRONMENTPROTO_H__

#include <QObject>
#include <QProcessEnvironment>
#include <QtScript>

Q_DECLARE_METATYPE(QProcessEnvironment)
Q_DECLARE_METATYPE(QProcessEnvironment*)

void setupQProcessEnvironmentProto(QScriptEngine *engine);
QScriptValue constructQProcessEnvironment(QScriptContext *context, QScriptEngine *engine);

class QProcessEnvironmentProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QProcessEnvironmentProto(QObject *parent);

    /*
    Q_INVOKABLE void            clear();
    Q_INVOKABLE bool            contains(const QString &name) const;
    Q_INVOKABLE void            insert(const QString &name, const QString &value);
    Q_INVOKABLE bool            isEmpty() const;
    Q_INVOKABLE void            remove(const QString &name);
    Q_INVOKABLE QStringList     toStringList() const;
    Q_INVOKABLE QString         value(const QString &name, const QString &defaultValue = QString()) const;
    */

    Q_INVOKABLE QProcessEnvironment systemEnvironment();
};

#endif
