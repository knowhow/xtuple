/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QVALIDATORPROTO_H__
#define __QVALIDATORPROTO_H__

#include <QValidator>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QValidator*)
//Q_DECLARE_METATYPE(QValidator)

void setupQValidatorProto(QScriptEngine *engine);
QScriptValue constructQValidator(QScriptContext *context, QScriptEngine *engine);

class QValidatorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QValidatorProto(QObject *parent);

/*
 * Leaving these out for now as we just need the ability to
 * pass this class without changing anything
    Q_INVOKABLE void fixup ( QString & input ) const;
    Q_INVOKABLE QLocale locale () const;
    Q_INVOKABLE void setLocale ( const QLocale & locale );
    Q_INVOKABLE State validate ( QString & input, int & pos ) const;
*/

};
#endif
