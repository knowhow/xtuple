/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qspaceritemproto.h"

#include "qsizepolicyproto.h"

#define DEBUG false

QScriptValue QSpacerItemtoScriptValue(QScriptEngine *engine, QSpacerItem* const &item)
{
  if (DEBUG) qDebug("QSpacerItemtoScriptValue(%p, %p) called", engine, item);
  QScriptValue result = engine->newObject();
  result.setData(engine->newVariant(qVariantFromValue(item)));
  return result;
}

void QSpacerItemfromScriptValue(const QScriptValue &obj, QSpacerItem* &item)
{
  if (DEBUG) qDebug("QSpacerItemfromScriptValue(obj, %p) called", item);
  item = obj.data().toVariant().value<QSpacerItem*>();
  if (DEBUG) qDebug("QSpacerItemfromScriptValue set %p", item);
}

void setupQSpacerItem(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QSpacerItemtoScriptValue, QSpacerItemfromScriptValue);

  QScriptValue proto = engine->newQObject(new QSpacerItemProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSpacerItem*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSpacerItem, proto);
  engine->globalObject().setProperty("QSpacerItem", constructor);
}

QScriptValue constructQSpacerItem(QScriptContext *context,
                                  QScriptEngine  *engine)
{
  if (DEBUG)
    qDebug("constructQSpacerItem() called with %d args",
           context->argumentCount());
  QSpacerItem *obj = 0;

  if (context->argumentCount() == 2 &&
      context->argument(0).isNumber() &&
      context->argument(1).isNumber())
    obj = new QSpacerItem(context->argument(0).toInt32(),
                          context->argument(1).toInt32());

  else if (context->argumentCount() == 3 &&
           context->argument(0).isNumber() &&
           context->argument(1).isNumber() &&
           context->argument(2).isNumber())
    obj = new QSpacerItem(context->argument(0).toInt32(),
                          context->argument(1).toInt32(),
                          qscriptvalue_cast<QSizePolicy::Policy>(context->argument(2)));

  else if (context->argumentCount() == 4 &&
           context->argument(0).isNumber() &&
           context->argument(1).isNumber() &&
           context->argument(2).isNumber() &&
           context->argument(3).isNumber())
    obj = new QSpacerItem(context->argument(0).toInt32(),
                          context->argument(1).toInt32(),
                          qscriptvalue_cast<QSizePolicy::Policy>(context->argument(2)),
                          qscriptvalue_cast<QSizePolicy::Policy>(context->argument(3)));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find appropriate QSpacerItem constructor");

  if (DEBUG)
    qDebug("constructQSpacerItem() returning %p", obj);
  return engine->toScriptValue(obj);
}

QSpacerItemProto::QSpacerItemProto(QObject *parent)
    : QObject(parent)
{
}

Qt::Alignment QSpacerItemProto::alignment() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->alignment();
  return (Qt::Alignment)0;
}

void QSpacerItemProto::changeSize(int w, int h, QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    item->changeSize(w, h, hPolicy, vPolicy);
}

Qt::Orientations QSpacerItemProto::expandingDirections() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->expandingDirections();
  return (Qt::Orientations)0;
}

QRect QSpacerItemProto::geometry() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->geometry();
  return QRect();
}

bool QSpacerItemProto::hasHeightForWidth() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->hasHeightForWidth();
  return false;
}

int QSpacerItemProto::heightForWidth(int w) const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->heightForWidth(w);
  return 0;
}

void QSpacerItemProto::invalidate()
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    item->invalidate();
}

bool QSpacerItemProto::isEmpty() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

QLayout *QSpacerItemProto::layout()
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->layout();
  return 0;
}

QSize QSpacerItemProto::maximumSize() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->maximumSize();
  return QSize();
}

int QSpacerItemProto::minimumHeightForWidth(int w) const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->minimumHeightForWidth(w);
  return 0;
}

QSize QSpacerItemProto::minimumSize() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->minimumSize();
  return QSize();
}

void QSpacerItemProto::setAlignment(Qt::Alignment alignment)
{
  if (DEBUG) qDebug("QSpacerItemProto::setAlignment(%d)", (int)alignment);
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    item->setAlignment(alignment);
}

void QSpacerItemProto::setGeometry(const QRect &r)
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    item->setGeometry(r);
}

QSize QSpacerItemProto::sizeHint() const
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->sizeHint();
  return QSize();
}

QSpacerItem *QSpacerItemProto::spacerItem()
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->spacerItem();
  return 0;
}

QWidget *QSpacerItemProto::widget()
{
  QSpacerItem *item = qscriptvalue_cast<QSpacerItem*>(thisObject());
  if (item)
    return item->widget();
  return 0;
}
