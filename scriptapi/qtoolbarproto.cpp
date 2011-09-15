/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtoolbarproto.h"

#include <QToolBar>

QScriptValue QToolBartoScriptValue(QScriptEngine *engine, QToolBar* const &item)
{
  return engine->newQObject(item);
}

void QToolBarfromScriptValue(const QScriptValue &obj, QToolBar* &item)
{
  item = qobject_cast<QToolBar*>(obj.toQObject());
}

void setupQToolBarProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QToolBartoScriptValue, QToolBarfromScriptValue);

  QScriptValue proto = engine->newQObject(new QToolBarProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QToolBar*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQToolBar,
                                                 proto);
  engine->globalObject().setProperty("QToolBar",  constructor);
}

QScriptValue constructQToolBar(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QToolBar *obj = 0;
  if (context->argumentCount() >= 2 && context->argument(1).isQObject())
    obj = new QToolBar(context->argument(0).toString(), qobject_cast<QWidget*>(context->argument(1).toQObject()));
  else if(context->argumentCount() == 1 && context->argument(0).isQObject())
    obj = new QToolBar(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else if(context->argumentCount() == 1)
    obj = new QToolBar(context->argument(0).toString());
  else
    obj = new QToolBar();
  return engine->toScriptValue(obj);
}

QToolBarProto::QToolBarProto(QObject *parent)
    : QObject(parent)
{
}

void QToolBarProto::addAction( QAction * action )
{
  QToolBar *item = qscriptvalue_cast<QToolBar*>(thisObject());
  if (item)
    item->addAction(action);
}

void QToolBarProto::addWidget( QWidget * widget )
{
  QToolBar *item = qscriptvalue_cast<QToolBar*>(thisObject());
  if (item)
    item->addWidget(widget);
}

QWidget *  QToolBarProto::widgetForAction ( QAction * action ) const
{
  QToolBar *item = qscriptvalue_cast<QToolBar*>(thisObject());
  if (item)
    return item->widgetForAction(action);

  return 0;
}

QAction *  QToolBarProto::insertWidget ( QAction * before, QWidget * widget )
{
  QToolBar *item = qscriptvalue_cast<QToolBar*>(thisObject());
  if (item)
    return item->insertWidget(before, widget);

  return 0;
}


