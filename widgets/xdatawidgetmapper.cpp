/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdatawidgetmapper.h"

XDataWidgetMapper::XDataWidgetMapper(QObject *parent) : 
  QDataWidgetMapper(parent)
{
}

XDataWidgetMapper::~XDataWidgetMapper()
{
}

void XDataWidgetMapper::addMapping(QWidget *widget, QString fieldName)
{
  QDataWidgetMapper::addMapping(widget, static_cast<QSqlTableModel*>(model())->fieldIndex(fieldName));
  emit newMapping(widget);
}

void XDataWidgetMapper::addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName)
{
  QDataWidgetMapper::addMapping(widget, static_cast<QSqlTableModel*>(model())->fieldIndex(fieldName), propertyName);
  emit newMapping(widget);
}

/* This overload allows for a default property that can mapped. 
   The value of the property at that mapping will be used when clear() is called.*/
void XDataWidgetMapper::addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName, const QByteArray &defaultName)
{
  QDataWidgetMapper::addMapping(widget, static_cast<QSqlTableModel*>(model())->fieldIndex(fieldName), propertyName);
  removeDefault(widget);
  widgetMap.append(WidgetMapper(widget, static_cast<QSqlTableModel*>(model())->fieldIndex(fieldName), defaultName));
  emit newMapping(widget);
}

void XDataWidgetMapper::clear()
{
    for (int i = 0; i < widgetMap.count(); ++i)
        clear(widgetMap[i]);
}

void XDataWidgetMapper::clear(WidgetMapper &m)
{
    if (m.widget.isNull())
        return;
        
    if (!m.property.isEmpty())
        model()->setData(model()->index(currentIndex(),m.section),m.widget->property(m.property));
}

QByteArray XDataWidgetMapper::mappedDefaultName(QWidget *widget)
{
  for (int i = 0; i < widgetMap.count(); ++i) 
    if (widgetMap.at(i).widget == widget)
    {
      return widgetMap.at(i).property;
    }
  return QByteArray();
}

void XDataWidgetMapper::removeDefault(QWidget *widget)
{
  for (int i = 0; i < widgetMap.count(); ++i) 
    if (widgetMap.at(i).widget == widget)
    {
      widgetMap.removeAt(i);
      return;
    }
}







