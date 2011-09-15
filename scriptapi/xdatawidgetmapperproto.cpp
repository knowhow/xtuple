/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdatawidgetmapperproto.h"
#include "xdatawidgetmapper.h"

void setupXDataWidgetMapperProto(QScriptEngine *engine)
{
  QScriptValue mapperproto = engine->newQObject(new XDataWidgetMapperProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XDataWidgetMapper*>(), mapperproto);
}

XDataWidgetMapperProto::XDataWidgetMapperProto(QObject *parent)
  : QObject(parent)
{
}

/* compiler errors (confusion between int and char*)
void XDataWidgetMapperProto::addMapping(QWidget *widget, int section)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, section);
}
*/

/* compiler errors (confusion between int and char*)
void XDataWidgetMapperProto::addMapping(QWidget *widget, int section, const QByteArray &propertyName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, section, propertyName);
}
*/

void XDataWidgetMapperProto::clearMapping()
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->clearMapping();
}

int XDataWidgetMapperProto::currentIndex() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->currentIndex();
  return -1;
}

QAbstractItemDelegate *XDataWidgetMapperProto::itemDelegate() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->itemDelegate();
  return 0;
}

QByteArray XDataWidgetMapperProto::mappedPropertyName(QWidget *widget) const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedPropertyName(widget);
  return QByteArray();
}

int XDataWidgetMapperProto::mappedSection(QWidget *widget) const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedSection(widget);
  return -1;
}

QWidget *XDataWidgetMapperProto::mappedWidgetAt(int section) const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedWidgetAt(section);
  return 0;
}

QAbstractItemModel *XDataWidgetMapperProto::model() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->model();
  return 0;
}

int XDataWidgetMapperProto::orientation() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return (int)(item->orientation());
  return 0;
}

void XDataWidgetMapperProto::removeMapping(QWidget *widget)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->removeMapping(widget);
}

QModelIndex XDataWidgetMapperProto::rootIndex() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->rootIndex();
  return QModelIndex();
}

void XDataWidgetMapperProto::setItemDelegate(QAbstractItemDelegate *delegate)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setItemDelegate(delegate);
}

void XDataWidgetMapperProto::setModel(QAbstractItemModel *model)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setModel(model);
}

void XDataWidgetMapperProto::setOrientation(int orientation)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setOrientation((Qt::Orientation)orientation);
}

void XDataWidgetMapperProto::setRootIndex(const QModelIndex &index)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setRootIndex(index);
}

void XDataWidgetMapperProto::setSubmitPolicy(int policy)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setSubmitPolicy((QDataWidgetMapper::SubmitPolicy)policy);
}

int XDataWidgetMapperProto::submitPolicy() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->submitPolicy();
  return -1;
}

QByteArray XDataWidgetMapperProto::mappedDefaultName(QWidget *widget)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedDefaultName(widget);
  return QByteArray();
}

void XDataWidgetMapperProto::addMapping(QWidget *widget, QString fieldName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, fieldName);
}

void XDataWidgetMapperProto::addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, fieldName, propertyName);
}

void XDataWidgetMapperProto::addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName, const QByteArray &defaultName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, fieldName, propertyName, defaultName);
}

void XDataWidgetMapperProto::removeDefault(QWidget *widget)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->removeDefault(widget);
}

QString XDataWidgetMapperProto::toString() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return QString("[XDataWidgetMapper(%1)]").arg(item->objectName());
  return QString("[XDataWidgetMapper(unknown)]");
}
