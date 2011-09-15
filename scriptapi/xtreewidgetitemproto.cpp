/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtreewidgetitemproto.h"
#include "xtreewidget.h"

void setupXTreeWidgetItemProto(QScriptEngine *engine)
{
  QScriptValue itemproto = engine->newQObject(new XTreeWidgetItemProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XTreeWidgetItem*>(), itemproto);
}

XTreeWidgetItemProto::XTreeWidgetItemProto(QObject *parent)
  : QObject(parent)
{
}

int XTreeWidgetItemProto::altId() const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->altId();
  return -1;
}

int XTreeWidgetItemProto::id()    const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->id();
  return -1;
}

void XTreeWidgetItemProto::setAltId(const int pId)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->setAltId(pId);
}

void XTreeWidgetItemProto::setId(const int pId)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->setId(pId);
}

XTreeWidgetItem *XTreeWidgetItemProto::child(int idx) const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->child(idx);
  return 0;
}    

int XTreeWidgetItemProto::id(const QString p)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->id(p);
  return -1;
}

QVariant XTreeWidgetItemProto::rawValue(const QString pName)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->rawValue(pName);
  return QVariant();
}

void XTreeWidgetItemProto::setText(int pColumn, const QVariant & pVariant)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    item->setText(pColumn, pVariant.toString());
}

void XTreeWidgetItemProto::setTextColor(int column, const QColor & color)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    item->setTextColor(column, color);
}

void XTreeWidgetItemProto::setTextColor(const QColor &pColor)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    item->setTextColor(pColor);
}

QString XTreeWidgetItemProto::text(int p) const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->text(p);
  return QString();
}

QString XTreeWidgetItemProto::text(const QString &pColumn) const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->text(pColumn);
  return QString();
}

QString XTreeWidgetItemProto::toString() const
{
  QString returnVal = QString("XTreeWidgetItem");
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
  {
    QStringList data;
    for (int i = 0; i < item->columnCount(); i++)
      data << item->text(i);
    returnVal += QString("(id = %1, altId = %2, data = [%3])")
      .arg(item->id()).arg(item->altId()).arg(data.join(", "));
  }
  else
    returnVal += "(unknown)";

  return "[" + returnVal + "]";
}
