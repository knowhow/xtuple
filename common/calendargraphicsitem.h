/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CALENDARGRAPHICSITEM_H__
#define __CALENDARGRAPHCISITEM_H__

#include <QObject>
#include <QGraphicsRectItem>
#include <QMap>

#include "graphicstextbuttonitem.h"

class CalendarControl;

class CalendarGraphicsItem : public QGraphicsRectItem, protected GraphicsButtonReceiver
{
  friend class GraphicsTextButtonItem;

  public:
    CalendarGraphicsItem(CalendarControl * = 0, QGraphicsItem * parent = 0);
    ~CalendarGraphicsItem();

    void setCalendarControl(CalendarControl * controller);
    CalendarControl * calendarControl() const;

    void setSelectedDay(const QDate &);
    QDate selectedDay() const { return _selectedDay; }

  protected:
    // protect these function that were public in the base class because we do not want
    // then to be changed outside this class
    void setRect(const QRectF & rectangle);
    void setRect(qreal x, qreal y, qreal width, qreal height);

    // reimplemented from GraphicsButtonReceiver
    void buttonMouseReleaseEvent(QGraphicsItem * origin, QGraphicsSceneMouseEvent * event);

    // reimplemeted from QGraphicsItem
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);

    QMap<QString,QGraphicsItem*> _items;
    QDate _selectedDay;

    CalendarControl * _controller;
};

#endif
