/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtreewidgetitemproto.h"

#include <QStringList>

void setupQTreeWidgetItemProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QTreeWidgetItemProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTreeWidgetItem*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QTreeWidgetItem>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQTreeWidgetItem,
                                                 proto);
  engine->globalObject().setProperty("QTreeWidgetItem",  constructor);

  QScriptValue widget = engine->newObject();

  widget.setProperty("ShowIndicator",                 QScriptValue(engine, QTreeWidgetItem::ShowIndicator), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DontShowIndicator",             QScriptValue(engine, QTreeWidgetItem::DontShowIndicator), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DontShowIndicatorWhenChildless",QScriptValue(engine, QTreeWidgetItem::DontShowIndicatorWhenChildless), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("Type",     QScriptValue(engine, QTreeWidgetItem::Type),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UserType", QScriptValue(engine, QTreeWidgetItem::UserType), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("OrderLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue constructQTreeWidgetItem(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  QTreeWidgetItem *obj = 0;

  if (context->argumentCount() == 0)
    obj = new QTreeWidgetItem();

  else if (context->argumentCount() == 1 && context->argument(0).isNumber())
    obj = new QTreeWidgetItem(context->argument(0).toInt32());

  else if (context->argumentCount() == 1 &&
           context->argument(0).isQObject() &&
           qobject_cast<QTreeWidget *>(context->argument(0).toQObject()))
    obj = new QTreeWidgetItem(qobject_cast<QTreeWidget *>(context->argument(0).toQObject()));

  else if (context->argumentCount() == 1 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()))
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()));

  /*
  else if (context->argumentCount() == 1 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QStringList *>(context->argument(0).toObject()))
    obj = new QTreeWidgetItem(qscriptvalue_cast<QStringList *>(context->argument(0).toObject()));

  else if (context->argumentCount() == 1 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QTreeWidgetItem &>(context->argument(0).toObject()))
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem &>(context->argument(0).toObject()));

  else if (context->argumentCount() == 2
           context->argument(0).isQObject() &&
           qobject_cast<QTreeWidget *>(context->argument(0).toQObject()) &&
           context->argument(1).isNumber())
    obj = new QTreeWidgetItem(qobject_cast<QTreeWidget *>(context->argument(0).toQObject()),
                              context->argument(1).toInt32());

  else if (context->argumentCount() == 2
           context->argument(0).isQObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()) &&
           context->argument(1).isNumber())
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()),
                              context->argument(1).toInt32());

  else if (context->argumentCount() == 2 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QStringList &>(context->argument(0).toObject()) &&
           context->argument(1).isNumber())
    obj = new QTreeWidgetItem(qscriptvalue_cast<QStringList &>(context->argument(0).toObject()),
                              context->argument(1).toInt32());

  else if (context->argumentCount() == 2 &&
           context->argument(0).isQObject() &&
           qobject_cast<QTreeWidget *>(context->argument(0).toQObject()) &&
           context->argument(1).isQObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()))
    obj = new QTreeWidgetItem(qobject_cast<QTreeWidget *>(context->argument(0).toQObject()),
                              qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()));

  else if (context->argumentCount() == 2 &&
           context->argument(0).isQObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()) &&
           context->argument(1).isQObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()))
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()),
                              qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()));

  else if (context->argumentCount() == 2 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()) &&
           context->argument(1).isObject() &&
           qscriptvalue_cast<QStringList &>(context->argument(1).toObject()))
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()),
                              qobject_cast<QStringList &>(context->argument(1).toObject()));

  else if (context->argumentCount() == 3
           context->argument(0).isQObject() &&
           qobject_cast<QTreeWidget *>(context->argument(0).toQObject()) &&
           qscriptvalue_cast<QStringList &>(context->argument(1).toObject()) &&
           context->argument(2).isNumber())
    obj = new QTreeWidgetItem(qobject_cast<QTreeWidget *>(context->argument(0).toQObject()),
                              qscriptvalue_cast<QStringList &>(context->argument(1).toObject()),
                              context->argument(2).toInt32());

  else if (context->argumentCount() == 3
           context->argument(0).isQObject() &&
           qobject_cast<QTreeWidget *>(context->argument(0).toQObject()) &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()) &&
           context->argument(2).isNumber())
    obj = new QTreeWidgetItem(qobject_cast<QTreeWidget *>(context->argument(0).toQObject()),
                              qscriptvalue_cast<QTreeWidgetItem &>(context->argument(1).toObject()),
                              context->argument(2).toInt32());

  else if (context->argumentCount() == 3 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()) &&
           context->argument(1).isObject() &&
           qscriptvalue_cast<QStringList &>(context->argument(1).toObject()) &&
           context->argument(2).isNumber())
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()),
                              qscriptvalue_cast<QStringList &>(context->argument(1).toObject()),
                              context->argument(2).toInt32());

  else if (context->argumentCount() == 3 &&
           context->argument(0).isObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()) &&
           context->argument(1).isObject() &&
           qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()) &&
           context->argument(2).isNumber())
    obj = new QTreeWidgetItem(qscriptvalue_cast<QTreeWidgetItem *>(context->argument(0).toObject()),
                              qscriptvalue_cast<QTreeWidgetItem *>(context->argument(1).toObject()),
                              context->argument(2).toInt32());
  */

  else
    obj = 0;

  return engine->toScriptValue(obj);
}

QTreeWidgetItemProto::QTreeWidgetItemProto(QObject *parent)
    : QObject(parent)
{
}

void QTreeWidgetItemProto::addChild(QTreeWidgetItem *child)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->addChild(child);
}

void QTreeWidgetItemProto::addChildren(const QList<QTreeWidgetItem *> &children)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->addChildren(children);
}

QBrush QTreeWidgetItemProto::background(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->background(column);
  return QBrush();
}

int QTreeWidgetItemProto::checkState(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->checkState(column);
  return 0;
}

QTreeWidgetItem *QTreeWidgetItemProto::child(int index)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->child(index);
  return 0;
}

int QTreeWidgetItemProto::childCount()	                const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->childCount();
  return 0;
}

int QTreeWidgetItemProto::childIndicatorPolicy()	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->childIndicatorPolicy();
  return 0;
}

QTreeWidgetItem *QTreeWidgetItemProto::clone()	                const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item ->clone();
  return 0;
}

int QTreeWidgetItemProto::columnCount()	                const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->columnCount();
  return 0;
}

QVariant QTreeWidgetItemProto::data(int column, int role)	const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->data(column, role);
  return QVariant();
}

int QTreeWidgetItemProto::flags()	                const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->flags();
  return 0;
}

QFont QTreeWidgetItemProto::font(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->font(column);
  return QFont();
}

QBrush QTreeWidgetItemProto::foreground(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->foreground(column);
  return QBrush();
}

QIcon QTreeWidgetItemProto::icon(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->icon(column);
  return QIcon();
}

int QTreeWidgetItemProto::indexOfChild(QTreeWidgetItem *child)	const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->indexOfChild(child);
  return -1;
}

void QTreeWidgetItemProto::insertChild(int index, QTreeWidgetItem *child)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->insertChild(index, child);
}

void QTreeWidgetItemProto::insertChildren(int index, const QList<QTreeWidgetItem *> &children)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->insertChildren(index, children);
}

bool QTreeWidgetItemProto::isDisabled()	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->isDisabled();
  return false;
}

bool QTreeWidgetItemProto::isExpanded()	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->isExpanded();
  return false;
}

bool QTreeWidgetItemProto::isFirstColumnSpanned()	const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->isFirstColumnSpanned();
  return false;
}

bool QTreeWidgetItemProto::isHidden()	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->isHidden();
  return false;
}

bool QTreeWidgetItemProto::isSelected()	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->isSelected();
  return false;
}

QTreeWidgetItem *QTreeWidgetItemProto::parent()	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item ->parent();
  return 0;
}

void QTreeWidgetItemProto::read(QDataStream &in)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->read(in);
}

void QTreeWidgetItemProto::removeChild(QTreeWidgetItem *child)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->removeChild(child);
}

void QTreeWidgetItemProto::setBackground(int column, const QBrush &brush)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setBackground(column, brush);
}

void QTreeWidgetItemProto::setCheckState(int column, int state)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setCheckState(column, (Qt::CheckState)state);
}

void QTreeWidgetItemProto::setChildIndicatorPolicy(int policy)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setChildIndicatorPolicy((QTreeWidgetItem::ChildIndicatorPolicy)policy);
}

void QTreeWidgetItemProto::setData(int column, int role, const QVariant &value)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setData(column, role, value);
}

void QTreeWidgetItemProto::setDisabled(bool disabled)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setDisabled(disabled);
}

void QTreeWidgetItemProto::setExpanded(bool expand)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setExpanded(expand);
}

void QTreeWidgetItemProto::setFirstColumnSpanned(bool span)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setFirstColumnSpanned(span);
}

void QTreeWidgetItemProto::setFlags(int flags)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setFlags((Qt::ItemFlags)flags);
}

void QTreeWidgetItemProto::setFont(int column, const QFont &font)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setFont(column, font);
}

void QTreeWidgetItemProto::setForeground(int column, const QBrush &brush)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setForeground(column, brush);
}

void QTreeWidgetItemProto::setHidden(bool hide)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setHidden(hide);
}

void QTreeWidgetItemProto::setIcon(int column, const QIcon &icon)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setIcon(column, icon);
}

void QTreeWidgetItemProto::setSelected(bool select)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setSelected(select);
}

void QTreeWidgetItemProto::setSizeHint(int column, const QSize &size)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setSizeHint(column, size);
}

void QTreeWidgetItemProto::setStatusTip(int column, const QString &statusTip)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setStatusTip(column, statusTip);
}

void QTreeWidgetItemProto::setText(int column, const QString &text)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setText(column, text);
}

void QTreeWidgetItemProto::setTextAlignment(int column, int alignment)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setTextAlignment(column, alignment);
}

void QTreeWidgetItemProto::setToolTip(int column, const QString &toolTip)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setToolTip(column, toolTip);
}

void QTreeWidgetItemProto::setWhatsThis(int column, const QString &whatsThis)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->setWhatsThis(column, whatsThis);
}

QSize QTreeWidgetItemProto::sizeHint(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->sizeHint(column);
  return QSize();
}

void QTreeWidgetItemProto::sortChildren(int column, int order)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->sortChildren(column, (Qt::SortOrder)order);
}

QString QTreeWidgetItemProto::statusTip(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->statusTip(column);
  return QString();
}

QTreeWidgetItem *QTreeWidgetItemProto::takeChild(int index)
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item ->takeChild(index);
  return 0;
}

QList<QTreeWidgetItem *> QTreeWidgetItemProto::takeChildren()
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->takeChildren();
  return QList<QTreeWidgetItem*>();
}

QString QTreeWidgetItemProto::text(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->text(column);
  return QString();
}

int QTreeWidgetItemProto::textAlignment(int column)	const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->textAlignment(column);
  return 0;
}

QString QTreeWidgetItemProto::toolTip(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->toolTip(column);
  return QString();
}

QTreeWidget *QTreeWidgetItemProto::treeWidget()	                const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->treeWidget();
  return 0;
}

int QTreeWidgetItemProto::type()	                        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->type();
  return 0;
}

QString QTreeWidgetItemProto::whatsThis(int column)	        const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return item->whatsThis(column);
  return QString();
}

void QTreeWidgetItemProto::write(QDataStream &out)	const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    item->write(out);
}

QString QTreeWidgetItemProto::toString()    	const
{
  QTreeWidgetItem *item = qscriptvalue_cast<QTreeWidgetItem*>(thisObject());
  if (item)
    return QString("[QTreeWidgetItem with %1 children and %2 columns]")
                   .arg(item->childCount())
                   .arg(item->columnCount());
  return QString();
}
