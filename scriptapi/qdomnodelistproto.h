/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMNODELISTPROTO_H__
#define __QDOMNODELISTPROTO_H__

#include <QDomNode>
#include <QDomNodeList>
#include <QObject>
#include <QtScript>


Q_DECLARE_METATYPE(QDomNodeList*)
Q_DECLARE_METATYPE(QDomNodeList)

void setupQDomNodeListProto(QScriptEngine *engine);
QScriptValue constructQDomNodeList(QScriptContext *context, QScriptEngine *engine);

class QDomNodeListProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomNodeListProto(QObject *parent);

    Q_INVOKABLE QDomNode at(int index)   const;
    Q_INVOKABLE int      count()         const;
    Q_INVOKABLE bool     isEmpty()       const;
    Q_INVOKABLE QDomNode item(int index) const;
    Q_INVOKABLE uint     length()        const;
    Q_INVOKABLE int      size()          const;
    Q_INVOKABLE QString  toString()      const;
};

#endif
