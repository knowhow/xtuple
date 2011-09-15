/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qgridlayoutproto.h"

#include <QString>
#include <QWidget>

#define DEBUG false

QScriptValue QGridLayouttoScriptValue(QScriptEngine *engine, QGridLayout* const &item)
{
  return engine->newQObject(item);
}

void QGridLayoutfromScriptValue(const QScriptValue &obj, QGridLayout* &item)
{
  item = qobject_cast<QGridLayout*>(obj.toQObject());
}

void setupQGridLayoutProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QGridLayouttoScriptValue, QGridLayoutfromScriptValue);

  QScriptValue proto = engine->newQObject(new QGridLayoutProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QGridLayout*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQGridLayout,
                                                 proto);
  engine->globalObject().setProperty("QGridLayout", constructor);
}

QScriptValue constructQGridLayout(QScriptContext *context,
                                  QScriptEngine  *engine)
{
  QGridLayout *obj = 0;
  if (context->argumentCount() == 0)
    obj = new QGridLayout();
  else if (context->argumentCount() > 0 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new QGridLayout(qscriptvalue_cast<QWidget*>(context->argument(0)));

  return engine->toScriptValue(obj);
}

QGridLayoutProto::QGridLayoutProto(QObject *parent)
    : QObject(parent)
{
}

bool QGridLayoutProto::activate()
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->activate();
  return false;
}

void QGridLayoutProto::addItem(QLayoutItem *item, int row, int column, int rowspan, int columnSpan, Qt::Alignment alignment)
{
  QGridLayout *griditem = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (DEBUG)
    qDebug("QGridLayoutProto(%p, %d, %d, %d, %d, %d) called",
           item, row, column, rowspan, columnSpan, (int)alignment);
  if (griditem)
    griditem->addItem(item, row, column, rowspan, columnSpan, alignment);
}

void QGridLayoutProto::addLayout(QLayout *layout, int row, int column, Qt::Alignment a)
{
  if (DEBUG)
    qDebug("QGridLayoutProto::addlayout(%p, %d, %d, %d)",
           layout, row, column, (int)a);
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item && layout)
    item->addLayout(layout, row, column, a);
}

void QGridLayoutProto::addLayout(QLayout *layout, int row, int column, int rowSpan, int columnSpan, Qt::Alignment a)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item && layout)
    item->addLayout(layout, row, column, rowSpan, columnSpan, a);
}

void QGridLayoutProto::addWidget(QWidget *widget, int row, int column, Qt::Alignment a)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->addWidget(widget, row, column, a);
}

void QGridLayoutProto::addWidget(QWidget *widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment a)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->addWidget(widget, fromRow, fromColumn, rowSpan, columnSpan, a);
}

QRect QGridLayoutProto::cellRect(int row, int column) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->cellRect(row, column);
  return QRect();
}

int QGridLayoutProto::columnCount() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->columnCount();
  return 0;
}

int QGridLayoutProto::columnMinimumWidth(int column) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->columnMinimumWidth(column);
  return 0;
}

int QGridLayoutProto::columnStretch(int column) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->columnStretch(column);
  return 0;
}

QRect QGridLayoutProto::contentsRect() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->contentsRect();
  return QRect();
}

int QGridLayoutProto::count() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

Qt::Orientations QGridLayoutProto::expandingDirections() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->expandingDirections();
  return Qt::Orientations();
}

void QGridLayoutProto::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->getContentsMargins(left, top, right, bottom);
}

void QGridLayoutProto::getItemPosition(int index, int *row, int *column, int *rowSpan, int *columnSpan)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->getItemPosition(index, row, column, rowSpan, columnSpan);
}

int QGridLayoutProto::horizontalSpacing() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->horizontalSpacing();
  return 0;
}

int QGridLayoutProto::indexOf(QWidget *widget) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->indexOf(widget);
  return 0;
}

void QGridLayoutProto::invalidate()
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->invalidate();
}

bool QGridLayoutProto::isEnabled() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->isEnabled();
  return false;
}

QLayoutItem *QGridLayoutProto::itemAt(int index) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->itemAt(index);
  return 0;
}

QLayoutItem *QGridLayoutProto::itemAtPosition(int row, int column) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->itemAtPosition(row, column);
  return 0;
}

QSize QGridLayoutProto::maximumSize() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->maximumSize();
  return QSize();
}

QWidget *QGridLayoutProto::menuBar() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->menuBar();
  return 0;
}

QSize QGridLayoutProto::minimumSize() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->minimumSize();
  return QSize();
}

Qt::Corner QGridLayoutProto::originCorner() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->originCorner();
  return (Qt::Corner)0;
}

QWidget *QGridLayoutProto::parentWidget() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->parentWidget();
  return 0;
}

void QGridLayoutProto::removeItem(QLayoutItem *item)
{
  QGridLayout *griditem = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (griditem)
    griditem->removeItem(item);
}

void QGridLayoutProto::removeWidget(QWidget *widget)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->removeWidget(widget);
}

int QGridLayoutProto::rowCount() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->rowCount();
  return 0;
}

int QGridLayoutProto::rowMinimumHeight(int row) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->rowMinimumHeight(row);
  return 0;
}

int QGridLayoutProto::rowStretch(int row) const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->rowStretch(row);
  return 0;
}

bool QGridLayoutProto::setAlignment(QWidget *w, Qt::Alignment alignment)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->setAlignment(w, alignment);
  return false;
}

void QGridLayoutProto::setAlignment(Qt::Alignment alignment)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setAlignment(alignment);
}

bool QGridLayoutProto::setAlignment(QLayout *l, Qt::Alignment alignment)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->setAlignment(l, alignment);
  return false;
}

void QGridLayoutProto::setColumnMinimumWidth(int column, int minSize)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setColumnMinimumWidth(column, minSize);
}

void QGridLayoutProto::setColumnStretch(int column, int stretch)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setColumnStretch(column, stretch);
}

void QGridLayoutProto::setContentsMargins(int left, int top, int right, int bottom)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setContentsMargins(left, top, right, bottom);
}

void QGridLayoutProto::setEnabled(bool enable)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setEnabled(enable);
}

void QGridLayoutProto::setHorizontalSpacing(int spacing)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setHorizontalSpacing(spacing);
}

void QGridLayoutProto::setMenuBar(QWidget *widget)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setMenuBar(widget);
}

void QGridLayoutProto::setOriginCorner(Qt::Corner corner)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setOriginCorner(corner);
}

void QGridLayoutProto::setRowMinimumHeight(int row, int minSize)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setRowMinimumHeight(row, minSize);
}

void QGridLayoutProto::setRowStretch(int row, int stretch)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setRowStretch(row, stretch);
}

void QGridLayoutProto::setSpacing(int spacing)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setSpacing(spacing);
}

void QGridLayoutProto::setVerticalSpacing(int spacing)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->setVerticalSpacing(spacing);
}

int QGridLayoutProto::spacing() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->spacing();
  return 0;
}

QLayoutItem *QGridLayoutProto::takeAt(int index)
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->takeAt(index);
  return 0;
}

void QGridLayoutProto::update()
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    item->update();
}

int QGridLayoutProto::verticalSpacing() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->verticalSpacing();
  return 0;
}

/*
QString QGridLayoutProto::toString() const
{
  QGridLayout *item = qscriptvalue_cast<QGridLayout*>(thisObject());
  if (item)
    return item->toString();
  return //QString();
}
*/
