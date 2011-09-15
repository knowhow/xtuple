/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XDATAWIDGETMAPPER_H

#define XDATAWIDGETMAPPER_H

#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include <QPointer>
#include <QWidget>

#include "widgets.h"

class XTUPLEWIDGETS_EXPORT XDataWidgetMapper : public QDataWidgetMapper
{
    Q_OBJECT

    public:
      XDataWidgetMapper(QObject *parent = 0);
      ~XDataWidgetMapper();
      
      virtual QByteArray mappedDefaultName(QWidget *widget);
      virtual void addMapping(QWidget *widget, QString fieldName);
      virtual void addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName);
      virtual void addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName, const QByteArray &defaultName);
      virtual void removeDefault(QWidget *widget);
      
    public slots:
      virtual void clear();
      
    signals:
      bool newMapping(QWidget *widget);

    private:
      struct WidgetMapper
      {
          inline WidgetMapper(QWidget *w, int c, const QByteArray &p)
              : widget(w),  section(c), property(p) {}

          QPointer<QWidget> widget;
          int section;
          QPersistentModelIndex currentIndex;
          QByteArray property;
      };

      QList<WidgetMapper> widgetMap;
      virtual void clear(WidgetMapper &m);

};

#endif
