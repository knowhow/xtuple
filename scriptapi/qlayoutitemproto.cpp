/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qlayoutitemproto.h"
#include "qspaceritemproto.h"

#define DEBUG false

QScriptValue QLayoutItemtoScriptValue(QScriptEngine *engine, QLayoutItem* const &item)
{
  if (DEBUG) qDebug("QLayoutItemtoScriptValue(%p, %p) called", engine, item);
  QScriptValue result = engine->newObject();
  result.setData(engine->newVariant(qVariantFromValue(item)));
  return result;
}

// TODO: why must we check for all possible value<> castings? this is fragile
void QLayoutItemfromScriptValue(const QScriptValue &obj, QLayoutItem* &item)
{
  if (DEBUG) qDebug("QLayoutItemfromScriptValue(obj, %p) called", item);
  item = obj.data().toVariant().value<QLayoutItem*>();
  if (! item)
    item = obj.data().toVariant().value<QSpacerItem*>();
  if (DEBUG) qDebug("QLayoutItemfromScriptValue set %p", item);
}

void setupQLayoutItemProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QLayoutItemtoScriptValue, QLayoutItemfromScriptValue);

  QScriptValue proto = engine->newQObject(new QLayoutItemProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QLayoutItem*>(), proto);
  if (DEBUG)
    qDebug("setupQLayoutItemProto() returning");
}

QLayoutItemProto::QLayoutItemProto(QObject *parent)
    : QObject(parent)
{
}

Qt::Alignment QLayoutItemProto::alignment() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->alignment();
  return (Qt::Alignment)0;
}

Qt::Orientations QLayoutItemProto::expandingDirections() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->expandingDirections();
  return (Qt::Orientations)0;
}

QRect QLayoutItemProto::geometry() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->geometry();
  return QRect();
}

bool QLayoutItemProto::hasHeightForWidth() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->hasHeightForWidth();
  return false;
}

int QLayoutItemProto::heightForWidth(int w) const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->heightForWidth(w);
  return 0;
}

void QLayoutItemProto::invalidate()
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    item->invalidate();
}

bool QLayoutItemProto::isEmpty() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

QLayout *QLayoutItemProto::layout()
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->layout();
  return 0;
}

QSize QLayoutItemProto::maximumSize() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->maximumSize();
  return QSize();
}

int QLayoutItemProto::minimumHeightForWidth(int w) const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->minimumHeightForWidth(w);
  return 0;
}

QSize QLayoutItemProto::minimumSize() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->minimumSize();
  return QSize();
}

void QLayoutItemProto::setAlignment(Qt::Alignment alignment)
{
  if (DEBUG) qDebug("QLayoutItemProto::setAlignment(%d)", (int)alignment);
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    item->setAlignment(alignment);
}

void QLayoutItemProto::setGeometry(const QRect &r)
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    item->setGeometry(r);
}

QSize QLayoutItemProto::sizeHint() const
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->sizeHint();
  return QSize();
}

QSpacerItem *QLayoutItemProto::spacerItem()
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->spacerItem();
  return 0;
}

QWidget *QLayoutItemProto::widget()
{
  QLayoutItem *item = qscriptvalue_cast<QLayoutItem*>(thisObject());
  if (item)
    return item->widget();
  return 0;
}
