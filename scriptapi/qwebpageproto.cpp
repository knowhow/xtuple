/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebpageproto.h"

#include <QWebPage>

QScriptValue QWebPagetoScriptValue(QScriptEngine *engine, QWebPage* const &item)
{
  return engine->newQObject(item);
}

void QWebPagefromScriptValue(const QScriptValue &obj, QWebPage* &item)
{
  item = qobject_cast<QWebPage*>(obj.toQObject());
}

void setupQWebPageProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebPagetoScriptValue, QWebPagefromScriptValue);

  QScriptValue proto = engine->newQObject(new QWebPageProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebPage*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebPage,
                                                 proto);
  engine->globalObject().setProperty("QWebPage",  constructor);

  constructor.setProperty("DontDelegateLinks",         QScriptValue(engine, QWebPage::DontDelegateLinks),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DelegateExternalLinks",       QScriptValue(engine, QWebPage::DelegateExternalLinks),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DelegateAllLinks",    QScriptValue(engine, QWebPage::DelegateAllLinks),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue constructQWebPage(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QWebPage *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QWebPage(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QWebPage();
  return engine->toScriptValue(obj);
}

QWebPageProto::QWebPageProto(QObject *parent)
    : QObject(parent)
{
}
