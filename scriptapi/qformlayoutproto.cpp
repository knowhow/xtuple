/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qformlayoutproto.h"

#include <QString>
#include <QWidget>

#define DEBUG false

QScriptValue QFormLayouttoScriptValue(QScriptEngine *engine, QFormLayout* const &item)
{
  return engine->newQObject(item);
}

void QFormLayoutfromScriptValue(const QScriptValue &obj, QFormLayout* &item)
{
  item = qobject_cast<QFormLayout*>(obj.toQObject());
}

void setupQFormLayoutProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QFormLayouttoScriptValue, QFormLayoutfromScriptValue);

  QScriptValue proto = engine->newQObject(new QFormLayoutProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QFormLayout*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQFormLayout,
                                                 proto);
  engine->globalObject().setProperty("QFormLayout", constructor);

  // enum QFormLayout::FieldGrowthPolicy
  constructor.setProperty("FieldsStayAtSizeHint",  QScriptValue(engine, QFormLayout::FieldsStayAtSizeHint),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ExpandingFieldsGrow",   QScriptValue(engine, QFormLayout::ExpandingFieldsGrow),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("AllNonFixedFieldsGrow", QScriptValue(engine, QFormLayout::AllNonFixedFieldsGrow), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QFormLayout::ItemRole
  constructor.setProperty("LabelRole",    QScriptValue(engine, QFormLayout::LabelRole),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("FieldRole",    QScriptValue(engine, QFormLayout::FieldRole),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("SpanningRole", QScriptValue(engine, QFormLayout::SpanningRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);

 // enum QFormLayout::RowWrapPolicy
  constructor.setProperty("DontWrapRows", QScriptValue(engine, QFormLayout::DontWrapRows), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("WrapLongRows", QScriptValue(engine, QFormLayout::WrapLongRows), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("WrapAllRows",  QScriptValue(engine, QFormLayout::WrapAllRows),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue constructQFormLayout(QScriptContext *context,
                                  QScriptEngine  *engine)
{
  QFormLayout *obj = 0;
  if (context->argumentCount() == 0)
    obj = new QFormLayout();
  else if (context->argumentCount() > 0 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new QFormLayout(qscriptvalue_cast<QWidget*>(context->argument(0)));

  return engine->toScriptValue(obj);
}

QFormLayoutProto::QFormLayoutProto(QObject *parent)
    : QObject(parent)
{
}

bool QFormLayoutProto::activate()
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->activate();
  return false;
}

void QFormLayoutProto::addItem(QLayoutItem *item)
{
  QFormLayout *formitem = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (DEBUG)
    qDebug("QFormLayoutProto::addItem(%p) called", item);
  if (formitem)
    formitem->addItem(item);
}

void QFormLayoutProto::addRow(QWidget *label, QWidget *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addRow(label, field);
}

void QFormLayoutProto::addRow(QWidget *label, QLayout *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addRow(label, field);
}

void QFormLayoutProto::addRow(const QString &labelText, QWidget *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addRow(labelText, field);
}

void QFormLayoutProto::addRow(const QString &labelText, QLayout *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addRow(labelText, field);
}

void QFormLayoutProto::addRow(QWidget *widget)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addRow(widget);
}

void QFormLayoutProto::addRow(QLayout *layout)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addRow(layout);
}

void QFormLayoutProto::addWidget(QWidget *widget)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->addWidget(widget);
}

QRect QFormLayoutProto::contentsRect() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->contentsRect();
  return QRect();
}

int QFormLayoutProto::count() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

Qt::Orientations QFormLayoutProto::expandingDirections() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->expandingDirections();
  return Qt::Orientations();
}

QFormLayout::FieldGrowthPolicy QFormLayoutProto::fieldGrowthPolicy()   const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->fieldGrowthPolicy();

  return QFormLayout::FieldsStayAtSizeHint;
}

Qt::Alignment QFormLayoutProto::formAlignment()   const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->formAlignment();

  return 0;
}

void QFormLayoutProto::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->getContentsMargins(left, top, right, bottom);
}

void QFormLayoutProto::getItemPosition(int index, int *rowPtr, QFormLayout::ItemRole *rolePtr) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->getItemPosition(index, rowPtr, rolePtr);
}

void QFormLayoutProto::getLayoutPosition(QLayout *layout, int *rowPtr, QFormLayout::ItemRole *rolePtr) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->getLayoutPosition(layout, rowPtr, rolePtr);
}

void QFormLayoutProto::getWidgetPosition(QWidget *widget, int *rowPtr, QFormLayout::ItemRole *rolePtr) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->getWidgetPosition(widget, rowPtr, rolePtr);
}

int QFormLayoutProto::horizontalSpacing() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->horizontalSpacing();
  return 0;
}

int QFormLayoutProto::indexOf(QWidget *widget) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->indexOf(widget);
  return 0;
}

void QFormLayoutProto::insertRow(int row, QWidget *label, QWidget *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->insertRow(row, label, field);
}

void QFormLayoutProto::insertRow(int row, QWidget *label, QLayout *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->insertRow(row, label, field);
}

void QFormLayoutProto::insertRow(int row, const QString &labelText, QWidget *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->insertRow(row, labelText, field);
}

void QFormLayoutProto::insertRow(int row, const QString &labelText, QLayout *field)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->insertRow(row, labelText, field);
}

void QFormLayoutProto::insertRow(int row, QWidget *widget)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->insertRow(row, widget);
}

void QFormLayoutProto::insertRow(int row, QLayout *layout)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->insertRow(row, layout);
}

void QFormLayoutProto::invalidate()
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->invalidate();
}

bool QFormLayoutProto::isEmpty() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QFormLayoutProto::isEnabled() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->isEnabled();
  return false;
}

QLayoutItem *QFormLayoutProto::itemAt(int index, QFormLayout::ItemRole role) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->itemAt(index, role);
  return 0;
}

QLayoutItem *QFormLayoutProto::itemAt(int index) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->itemAt(index);
  return 0;
}

Qt::Alignment QFormLayoutProto::labelAlignment()              const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->labelAlignment();
  return 0;
}

QWidget *QFormLayoutProto::labelForField(QWidget *field) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->labelForField(field);
  return 0;
}

QWidget *QFormLayoutProto::labelForField(QLayout *field) const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->labelForField(field);
  return 0;
}


QSize QFormLayoutProto::maximumSize() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->maximumSize();
  return QSize();
}

QWidget *QFormLayoutProto::menuBar() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->menuBar();
  return 0;
}

QSize QFormLayoutProto::minimumSize() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->minimumSize();
  return QSize();
}

QWidget *QFormLayoutProto::parentWidget() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->parentWidget();
  return 0;
}

void QFormLayoutProto::removeItem(QLayoutItem *item)
{
  QFormLayout *formitem = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (formitem)
    formitem->removeItem(item);
}

void QFormLayoutProto::removeWidget(QWidget *widget)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->removeWidget(widget);
}

int QFormLayoutProto::rowCount() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->rowCount();
  return 0;
}

QFormLayout::RowWrapPolicy QFormLayoutProto::rowWrapPolicy() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->rowWrapPolicy();
  return QFormLayout::DontWrapRows;
}

bool QFormLayoutProto::setAlignment(QWidget *w, Qt::Alignment alignment)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->setAlignment(w, alignment);
  return false;
}

void QFormLayoutProto::setAlignment(Qt::Alignment alignment)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setAlignment(alignment);
}

bool QFormLayoutProto::setAlignment(QLayout *l, Qt::Alignment alignment)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->setAlignment(l, alignment);
  return false;
}

void QFormLayoutProto::setContentsMargins(int left, int top, int right, int bottom)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setContentsMargins(left, top, right, bottom);
}

void QFormLayoutProto::setEnabled(bool enable)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setEnabled(enable);
}

void QFormLayoutProto::setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy policy)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setFieldGrowthPolicy(policy);
}

void QFormLayoutProto::setFormAlignment(Qt::Alignment alignment)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setFormAlignment(alignment);
}

void QFormLayoutProto::setHorizontalSpacing(int spacing)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setHorizontalSpacing(spacing);
}

void QFormLayoutProto::setItem(int row, QFormLayout::ItemRole role, QLayoutItem *item)
{
  QFormLayout *i = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (i)
    i->setItem(row, role, item);
}

void QFormLayoutProto::setLabelAlignment(Qt::Alignment alignment)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setLabelAlignment(alignment);
}

void QFormLayoutProto::setLayout(int row, QFormLayout::ItemRole role, QLayout *layout)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setLayout(row, role, layout);
}

void QFormLayoutProto::setMenuBar(QWidget *widget)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setMenuBar(widget);
}

void QFormLayoutProto::setRowWrapPolicy(QFormLayout::RowWrapPolicy policy)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setRowWrapPolicy(policy);
}

void QFormLayoutProto::setSpacing(int spacing)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setSpacing(spacing);
}

void QFormLayoutProto::setVerticalSpacing(int spacing)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setVerticalSpacing(spacing);
}

void QFormLayoutProto::setWidget(int row, QFormLayout::ItemRole role, QWidget *widget)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->setWidget(row, role, widget);
}

int QFormLayoutProto::spacing() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->spacing();
  return 0;
}

QLayoutItem *QFormLayoutProto::takeAt(int index)
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->takeAt(index);
  return 0;
}

void QFormLayoutProto::update()
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    item->update();
}

int QFormLayoutProto::verticalSpacing() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return item->verticalSpacing();
  return 0;
}

QString QFormLayoutProto::toString() const
{
  QFormLayout *item = qscriptvalue_cast<QFormLayout*>(thisObject());
  if (item)
    return tr("[ QFormLayout %1 ]").arg(item->objectName());
  return QString();
}
