/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMPROCESSINGINSTRUCTIONPROTO_H__
#define __QDOMPROCESSINGINSTRUCTIONPROTO_H__

#include <QDomProcessingInstruction>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QDomProcessingInstruction*)
Q_DECLARE_METATYPE(QDomProcessingInstruction)

void setupQDomProcessingInstructionProto(QScriptEngine *engine);
QScriptValue constructQDomProcessingInstruction(QScriptContext *context, QScriptEngine *engine);

class QDomProcessingInstructionProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomProcessingInstructionProto(QObject *parent);

    Q_INVOKABLE QString data() const;
    Q_INVOKABLE int     nodeType() const;
    Q_INVOKABLE void    setData(const QString& d);
    Q_INVOKABLE QString target() const;
    Q_INVOKABLE QString toString() const;
};

#endif
