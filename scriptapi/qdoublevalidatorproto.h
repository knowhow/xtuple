/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOUBLEVALIDATORPROTO_H__
#define __QDOUBLEVALIDATORPROTO_H__

#include <QDoubleValidator>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QDoubleValidator*)
//Q_DECLARE_METATYPE(QDoubleValidator)

void setupQDoubleValidatorProto(QScriptEngine *engine);
QScriptValue constructQDoubleValidator(QScriptContext *context, QScriptEngine *engine);

class QDoubleValidatorProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDoubleValidatorProto(QObject *parent);

/*
 * Leaving these out for now as we just need the ability to
 * pass this class without changing anything
    Q_INVOKABLE double bottom () const;
    Q_INVOKABLE int decimals () const;
    Q_INVOKABLE Notation notation () const;
    Q_INVOKABLE void setBottom ( double );
    Q_INVOKABLE void setDecimals ( int );
    Q_INVOKABLE void setNotation ( Notation );
    Q_INVOKABLE void setRange ( double minimum, double maximum, int decimals = 0 );
    Q_INVOKABLE void setTop ( double );
    Q_INVOKABLE double top () const;
    Q_INVOKABLE virtual QValidator::State validate ( QString & input, int & pos ) const;
*/

};
#endif
