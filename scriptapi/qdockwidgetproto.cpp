/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdockwidgetproto.h"

#include <QAction>
#include <QDockWidget>

QScriptValue QDockWidgettoScriptValue(QScriptEngine *engine, QDockWidget* const &item)
{
  return engine->newQObject(item);
}

void QDockWidgetfromScriptValue(const QScriptValue &obj, QDockWidget* &item)
{
  item = qobject_cast<QDockWidget*>(obj.toQObject());
}

void setupQDockWidgetProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QDockWidgettoScriptValue, QDockWidgetfromScriptValue);

  QScriptValue proto = engine->newQObject(new QDockWidgetProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDockWidget*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQDockWidget,
                                                 proto);
  engine->globalObject().setProperty("QDockWidget",  constructor);
}

QScriptValue constructQDockWidget(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QDockWidget *obj = 0;
  if (context->argumentCount() == 3)
  {
    obj = new QDockWidget(context->argument(0).toString(),
                          qobject_cast<QWidget*>(context->argument(0).toQObject()),
                          (Qt::WindowFlags)(context->argument(1).toInt32()));
  }
  else if (context->argumentCount() == 2)
  {
    obj = new QDockWidget(qobject_cast<QWidget*>(context->argument(0).toQObject()),
                          (Qt::WindowFlags)(context->argument(1).toInt32()));
  }
  else if (context->argumentCount() == 1)
  {
    obj = new QDockWidget(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  }
  else
    obj = new QDockWidget();
  return engine->toScriptValue(obj);
}

QDockWidgetProto::QDockWidgetProto(QObject *parent)
    : QObject(parent)
{
}

void QDockWidgetProto::setTitleBarWidget ( QWidget * widget )
{
  QDockWidget *item = qscriptvalue_cast<QDockWidget*>(thisObject());
  if (item)
    item->setTitleBarWidget(widget);
}

void QDockWidgetProto::setWidget ( QWidget * widget )
{
  QDockWidget *item = qscriptvalue_cast<QDockWidget*>(thisObject());
  if (item)
    item->setWidget(widget);
}

QWidget* QDockWidgetProto::titleBarWidget () const
{
  QDockWidget *item = qscriptvalue_cast<QDockWidget*>(thisObject());
  if (item)
    return item->titleBarWidget();
  return 0;
}

QAction* QDockWidgetProto::toggleViewAction () const
{
  QDockWidget *item = qscriptvalue_cast<QDockWidget*>(thisObject());
  if (item)
    return item->toggleViewAction();
  return 0;
}

QWidget* QDockWidgetProto::widget() const
{
  QDockWidget *item = qscriptvalue_cast<QDockWidget*>(thisObject());
  if (item)
    return item->widget();
  return 0;
}
