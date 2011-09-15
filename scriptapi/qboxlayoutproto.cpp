/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qboxlayoutproto.h"

#include <QString>
#include <QWidget>
#include <QtScript>

#define DEBUG false

QScriptValue QBoxLayouttoScriptValue(QScriptEngine *engine, QBoxLayout* const &item)
{
  return engine->newQObject(item);
}

void QBoxLayoutfromScriptValue(const QScriptValue &obj, QBoxLayout* &item)
{
  item = qobject_cast<QBoxLayout*>(obj.toQObject());
}

void setupQBoxLayoutProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QBoxLayouttoScriptValue, QBoxLayoutfromScriptValue);

  QScriptValue proto = engine->newQObject(new QBoxLayoutProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QBoxLayout*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQBoxLayout,
                                                 proto);
  engine->globalObject().setProperty("QBoxLayout",  constructor);
  constructor.setProperty("LeftToRight", QScriptValue(engine, QBoxLayout::LeftToRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("RightToLeft", QScriptValue(engine, QBoxLayout::RightToLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("TopToBottom", QScriptValue(engine, QBoxLayout::TopToBottom), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("BottomToTop", QScriptValue(engine, QBoxLayout::BottomToTop), QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue constructQBoxLayout(QScriptContext *context,
                                 QScriptEngine  *engine)
{
  QBoxLayout *obj = 0;
  if (context->argumentCount() == 1)
  {
    if (DEBUG) qDebug("QBoxLayout(%s)",
                      qPrintable(context->argument(0).toString()));
    obj = new QBoxLayout((QBoxLayout::Direction)(context->argument(0).toInt32()));
  }
  else if (context->argumentCount() > 1 &&
           qobject_cast<QWidget*>(context->argument(1).toQObject()))
  {
    if (DEBUG) qDebug("QBoxLayout(%s, %p)",
                      qPrintable(context->argument(0).toString()),
                      context->argument(1).toQObject());
    obj = new QBoxLayout((QBoxLayout::Direction)(context->argument(0).toInt32()),
                         qobject_cast<QWidget*>(context->argument(1).toQObject()));
  }
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QBoxLayout constructor");

  if (DEBUG)
    qDebug("constructBoxLayout returning %p", obj);
  return engine->toScriptValue(obj);
}

QBoxLayoutProto::QBoxLayoutProto(QObject *parent)
    : QObject(parent)
{
}

bool QBoxLayoutProto::activate()
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->activate();
  return false;
}

void QBoxLayoutProto::addItem(QLayoutItem *item)
{
  QBoxLayout *boxitem = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (boxitem)
    boxitem->addItem(item);
}

void QBoxLayoutProto::addLayout(QLayout *layout, int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->addLayout(layout, stretch);
}

void QBoxLayoutProto::addSpacerItem(QSpacerItem *spacerItem)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->addSpacerItem(spacerItem);
}

void QBoxLayoutProto::addSpacing(int size)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->addSpacing(size);
}

void QBoxLayoutProto::addStretch(int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->addStretch(stretch);
}

void QBoxLayoutProto::addStrut(int size)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->addStrut(size);
}

void QBoxLayoutProto::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->addWidget(widget, stretch, alignment);
}

QRect QBoxLayoutProto::contentsRect() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->contentsRect();
  return QRect();
}

int QBoxLayoutProto::count() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

QBoxLayout::Direction QBoxLayoutProto::direction() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->direction();
  return QBoxLayout::LeftToRight;
}

Qt::Orientations QBoxLayoutProto::expandingDirections() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->expandingDirections();
  return Qt::Horizontal;
}

void QBoxLayoutProto::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->getContentsMargins(left, top, right, bottom);
}

int QBoxLayoutProto::indexOf(QWidget *widget) const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->indexOf(widget);
  return 0;
}

void QBoxLayoutProto::insertLayout(int index, QLayout *layout, int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->insertLayout(index, layout, stretch);
}

void QBoxLayoutProto::insertSpacerItem(int index, QSpacerItem *spacerItem)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->insertSpacerItem(index, spacerItem);
}

void QBoxLayoutProto::insertSpacing(int index, int size)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->insertSpacing(index, size);
}

void QBoxLayoutProto::insertStretch(int index, int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->insertStretch(index, stretch);
}

void QBoxLayoutProto::insertWidget(int index, QWidget *widget, int stretch, Qt::Alignment alignment)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->insertWidget(index, widget, stretch, alignment);
}

void QBoxLayoutProto::invalidate()
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->invalidate();
}

bool QBoxLayoutProto::isEnabled() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->isEnabled();
  return false;
}

QLayoutItem *QBoxLayoutProto::itemAt(int index) const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->itemAt(index);
  return 0;
}

QSize QBoxLayoutProto::maximumSize() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->maximumSize();
  return QSize();
}

QWidget *QBoxLayoutProto::menuBar() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->menuBar();
  return 0;
}

QSize QBoxLayoutProto::minimumSize() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->minimumSize();
  return QSize();
}

QWidget *QBoxLayoutProto::parentWidget() const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->parentWidget();
  return 0;
}

void QBoxLayoutProto::removeItem(QLayoutItem *item)
{
  QBoxLayout *boxitem = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (boxitem)
    boxitem->removeItem(item);
}

void QBoxLayoutProto::removeWidget(QWidget *widget)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->removeWidget(widget);
}

void QBoxLayoutProto::setAlignment(Qt::Alignment alignment)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->setAlignment(alignment);
}

void QBoxLayoutProto::setContentsMargins(int left, int top, int right, int bottom)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->setContentsMargins(left, top, right, bottom);
}

void QBoxLayoutProto::setDirection(int direction)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->setDirection((QBoxLayout::Direction)direction);
}

void QBoxLayoutProto::setEnabled(bool enable)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->setEnabled(enable);
}

void QBoxLayoutProto::setMenuBar(QWidget *widget)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->setMenuBar(widget);
}

void QBoxLayoutProto::setStretch(int index, int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->setStretch(index, stretch);
}

bool QBoxLayoutProto::setAlignment(QLayout *l, Qt::Alignment alignment)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->setAlignment(l, alignment);
  return false;
}

bool QBoxLayoutProto::setAlignment(QWidget *w, Qt::Alignment alignment)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->setAlignment(w, alignment);
  return false;
}

bool QBoxLayoutProto::setStretchFactor(QLayout *layout, int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->setStretchFactor(layout, stretch);
  return false;
}

bool QBoxLayoutProto::setStretchFactor(QWidget *widget, int stretch)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->setStretchFactor(widget, stretch);
  return false;
}

int QBoxLayoutProto::stretch(int index) const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->stretch(index);
  return 0;
}

QLayoutItem *QBoxLayoutProto::takeAt(int index)
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->takeAt(index);
  return 0;
}

void QBoxLayoutProto::update()
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    item->update();
}

/*
QString QBoxLayoutProto::toString()     const
{
  QBoxLayout *item = qscriptvalue_cast<QBoxLayout*>(thisObject());
  if (item)
    return item->toString()     const;
  return //QString();
}
*/
