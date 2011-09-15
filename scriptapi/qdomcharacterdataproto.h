/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMCHARACTERDATAPROTO_H__
#define __QDOMCHARACTERDATAPROTO_H__

#include <QDomCharacterData>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomCharacterData*)
Q_DECLARE_METATYPE(QDomCharacterData)

void setupQDomCharacterDataProto(QScriptEngine *engine);
QScriptValue constructQDomCharacterData(QScriptContext *context, QScriptEngine *engine);

class QDomCharacterDataProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomCharacterDataProto(QObject *parent);

    Q_INVOKABLE void    appendData(const QString& arg);
    Q_INVOKABLE QString data()     const;
    Q_INVOKABLE void    deleteData(unsigned long offset, unsigned long count);
    Q_INVOKABLE void    insertData(unsigned long offset, const QString& arg);
    Q_INVOKABLE uint    length()   const;
    Q_INVOKABLE int     nodeType() const;
    Q_INVOKABLE void    replaceData(unsigned long offset, unsigned long count, const QString& arg);
    Q_INVOKABLE void    setData(const QString&);
    Q_INVOKABLE QString substringData(unsigned long offset, unsigned long count);
    Q_INVOKABLE QString toString() const;
};

#endif
