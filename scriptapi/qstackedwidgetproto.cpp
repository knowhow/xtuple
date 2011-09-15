/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qstackedwidgetproto.h"

QScriptValue QStackedWidgettoScriptValue(QScriptEngine *engine, QStackedWidget* const &item)
{
  return engine->newQObject(item);
}

void QStackedWidgetfromScriptValue(const QScriptValue &obj, QStackedWidget* &item)
{
  item = qobject_cast<QStackedWidget*>(obj.toQObject());
}

void setupQStackedWidgetProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QStackedWidgetProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QStackedWidget*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QStackedWidget>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQStackedWidget,
                                                 proto);
  engine->globalObject().setProperty("QStackedWidget",  constructor);
}

QScriptValue constructQStackedWidget(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QStackedWidget *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QStackedWidget(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QStackedWidget();
  return engine->toScriptValue(obj);
}

QStackedWidgetProto::QStackedWidgetProto(QObject *parent)
    : QObject(parent)
{
}

int QStackedWidgetProto::addWidget(QWidget *widget)
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->addWidget(widget);
  return -1;
}

int QStackedWidgetProto::count()                  const
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

int QStackedWidgetProto::currentIndex()           const
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->currentIndex();
  return 0;
}

QWidget *QStackedWidgetProto::currentWidget()          const
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->currentWidget();
  return 0;
}

int QStackedWidgetProto::indexOf(QWidget *widget) const
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->indexOf(widget);
  return 0;
}

int QStackedWidgetProto::insertWidget(int index, QWidget *widget)
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->insertWidget(index, widget);
  return 0;
}

void QStackedWidgetProto::removeWidget(QWidget *widget)
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    item->removeWidget(widget);
}

QWidget *QStackedWidgetProto::widget(int index)                 const
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->widget(index);
  return 0;
}

/*
QString QStackedWidgetProto::toString()                const
{
  QStackedWidget *item = qscriptvalue_cast<QStackedWidget*>(thisObject());
  if (item)
    return item->toString()                const;
  return QString();
}
*/
