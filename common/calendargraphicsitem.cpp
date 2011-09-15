/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDate>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QApplication>

#include <QDebug>

#include "calendargraphicsitem.h"
#include "graphicstextbuttonitem.h"
#include "calendarcontrol.h"

static const qreal __dpi = 72.0;
static const qreal __width = 504.0; // 7.0 * __dpi
static const qreal __height = 594.0; // 8.25 * __dpi
static const qreal __titleHeight = 36.0; // 0.5 * __dpi

static const QFont monthfont("Lucida", 24, QFont::Bold);
static const QFont navfont("lucida", 20, QFont::Bold);
static const QFont wdayfont("Lucida", 8);
static const QFont dayfont("Lucida", 20, QFont::Bold);
static const QFont notesfont("Lucida", 16);

static const QBrush blackFill(Qt::SolidPattern);
static const QBrush whiteFill("white");
static const QBrush nonMonthFill("lightgray");
static const QBrush nonMonthDayFill("gray");
static const QBrush weekendFill("khaki");
static const QBrush todayFill("paleturquoise");
static const QBrush selectedFill("lightgreen");

static const QPen noPen(Qt::NoPen);
static const QPen defaultPen;

CalendarGraphicsItem::CalendarGraphicsItem(CalendarControl * cc, QGraphicsItem * parent)
  : QGraphicsRectItem(0.0, 0.0, __width, __height, parent)
{
  _controller = cc;

  QDate today = QDate::currentDate();
  QDate firstMonthDay = QDate(today.year(), today.month(), 1);
  QDate firstCalendarDay = firstMonthDay.addDays(firstMonthDay.dayOfWeek() * -1);
  if(firstMonthDay.dayOfWeek() < 2)
    firstCalendarDay = firstCalendarDay.addDays(-7);

  _selectedDay = today;

  QString prev = QObject::tr("<");
  QString prevprev = QObject::tr("<<");
  QString next = QObject::tr(">");
  QString nextnext = QObject::tr(">>");

  QGraphicsSimpleTextItem * textItem;
  QGraphicsRectItem * rectItem;

  rectItem = new QGraphicsRectItem(0.0, 0.0, __width, __titleHeight, this);
  rectItem->setBrush(blackFill);
  _items.insert("titleBackground", rectItem);

  textItem = new QGraphicsSimpleTextItem(today.toString("MMMM yyyy"), this);
  textItem->setFont(monthfont);
  textItem->setZValue(2);
  textItem->setBrush(whiteFill);
  QPointF ct = rectItem->boundingRect().center();
  QRectF rt = textItem->boundingRect();
  textItem->setPos(ct.x() - (rt.width() / 2),
                   ct.y() - (rt.height() / 2));
  _items.insert("title", textItem);

  qreal offset = 0.1 * __dpi;
  qreal nw = 0;
  GraphicsTextButtonItem * tbtnItem = 0;
  tbtnItem = new GraphicsTextButtonItem(prevprev, this);
  tbtnItem->setFont(navfont);
  tbtnItem->setZValue(3);
  tbtnItem->setBrush(Qt::lightGray);
  tbtnItem->setRolloverBrush(Qt::green);
  tbtnItem->scale(0.5, 1);
  tbtnItem->setReceiver(this);
  rt = tbtnItem->boundingRect();
  tbtnItem->setPos(offset, ct.y() - (rt.height() / 2));
  nw = rt.width();
  _items.insert("fastrewind", tbtnItem);

  tbtnItem = new GraphicsTextButtonItem(nextnext, this);
  tbtnItem->setFont(navfont);
  tbtnItem->setZValue(3);
  tbtnItem->setBrush(Qt::lightGray);
  tbtnItem->setRolloverBrush(Qt::green);
  tbtnItem->scale(0.5, 1);
  tbtnItem->setReceiver(this);
  rt = tbtnItem->boundingRect();
  tbtnItem->setPos(rectItem->boundingRect().right() - offset - (rt.width() / 2), ct.y() - (rt.height() / 2));
  nw = qMax(nw, rt.width()) / 2;
  offset += (nw * 1.5);
  _items.insert("fastforward", tbtnItem);

  tbtnItem = new GraphicsTextButtonItem(prev, this);
  tbtnItem->setFont(navfont);
  tbtnItem->setZValue(3);
  tbtnItem->setBrush(Qt::lightGray);
  tbtnItem->setRolloverBrush(Qt::green);
  tbtnItem->scale(0.5, 1);
  tbtnItem->setReceiver(this);
  rt = tbtnItem->boundingRect();
  tbtnItem->setPos(offset, ct.y() - (rt.height() / 2));
  _items.insert("rewind", tbtnItem);

  tbtnItem = new GraphicsTextButtonItem(next, this);
  tbtnItem->setFont(navfont);
  tbtnItem->setZValue(3);
  tbtnItem->setBrush(Qt::lightGray);
  tbtnItem->setRolloverBrush(Qt::green);
  tbtnItem->scale(0.5, 1);
  tbtnItem->setReceiver(this);
  rt = tbtnItem->boundingRect();
  tbtnItem->setPos(rectItem->boundingRect().right() - offset - (rt.width() / 2), ct.y() - (rt.height() / 2));
  _items.insert("forward", tbtnItem);

  QDate date;
  qreal dayWidth = __width / 7.0;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for(int wday = 0; wday < 7; wday++)
  {
    for(int week = 0; week < 6; week++)
    {
      date = firstCalendarDay.addDays((7 * week) + wday);

      if(0 == week)
      {
        rectItem = new QGraphicsRectItem(wday * dayWidth, 0.5 * __dpi, dayWidth, 0.25 * __dpi, this);
        _items.insert(QString("weekday%1").arg(week), rectItem);
        textItem = new QGraphicsSimpleTextItem(date.toString("dddd"), this);
        textItem->setFont(wdayfont);
        textItem->setZValue(2);
        ct = rectItem->boundingRect().center();
        rt = textItem->boundingRect();
        textItem->setPos(ct.x() - (rt.width() / 2),
                         ct.y() - (rt.height() / 2));
        _items.insert(QString("weekday%1Text").arg(week), textItem);
      }

      QBrush fill;
      QBrush dayFill = blackFill;
      if(date == _selectedDay)
        fill = selectedFill;
      else if(date.month() != today.month())
      {
        fill = nonMonthFill;
        dayFill = nonMonthDayFill;
      }
      else if(date == today)
        fill = todayFill;
      else if(date.dayOfWeek() > 5)
        fill = weekendFill;

      rectItem = new QGraphicsRectItem(wday * dayWidth, (0.75  + (1.25 * week)) * __dpi, dayWidth, 1.25 * __dpi, this);
      rectItem->setBrush(fill);
      _items.insert(QString("day%1").arg((7 * week) + wday), rectItem);
      rt = QRectF(rectItem->pos(), rectItem->boundingRect().size());
      double offset = rt.width() / 3;
      textItem = new QGraphicsSimpleTextItem(QString::number(date.day()), this);
      textItem->setFont(dayfont);
      textItem->setZValue(2);
      textItem->setBrush(dayFill);
      textItem->setPos(rectItem->boundingRect().topLeft());
      textItem->moveBy(5.0, 5.0);
      _items.insert(QString("day%1Number").arg((7 * week) + wday), textItem);

      rt = QRectF(textItem->pos(), textItem->boundingRect().size());

      QString additionalText;
      if(_controller)
        additionalText = _controller->contents(date);
      textItem = new QGraphicsSimpleTextItem(additionalText, this);
      textItem->setFont(notesfont);
      textItem->setZValue(2);
      textItem->setBrush(dayFill);
      textItem->setPos(rt.left() + offset,
                       rt.top() + (rt.height() * 1.5));
      _items.insert(QString("day%1Text").arg((7 * week) + wday), textItem);
    }
  }
  QApplication::restoreOverrideCursor();
}

CalendarGraphicsItem::~CalendarGraphicsItem()
{
}

void CalendarGraphicsItem::setCalendarControl(CalendarControl * controller)
{
  _controller = controller;
}

CalendarControl * CalendarGraphicsItem::calendarControl() const
{
  return _controller;
}

void CalendarGraphicsItem::setSelectedDay(const QDate & sDate)
{
  //if(sDate == _selectedDay)
  //  return;

  _selectedDay = sDate;

  QDate today = QDate::currentDate();
  QDate firstMonthDay = QDate(_selectedDay.year(), _selectedDay.month(), 1);
  QDate firstCalendarDay = firstMonthDay.addDays(firstMonthDay.dayOfWeek() * -1);
  if(firstMonthDay.dayOfWeek() < 2)
    firstCalendarDay = firstCalendarDay.addDays(-7);

  QGraphicsRectItem * titleBackground = static_cast<QGraphicsRectItem*>(_items["titleBackground"]);
  QGraphicsSimpleTextItem * title = static_cast<QGraphicsSimpleTextItem*>(_items["title"]);
  if(title && titleBackground)
  { 
    title->setText(_selectedDay.toString("MMMM yyyy"));
    QPointF ct = titleBackground->boundingRect().center();
    QRectF rt = title->boundingRect();
    title->setPos(ct.x() - (rt.width() / 2),
                     ct.y() - (rt.height() / 2));
  }

  QDate date;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for(int wday = 0; wday < 42; wday++)
  {
    date = firstCalendarDay.addDays(wday);

    QBrush fill;
    QBrush dayFill = blackFill;
    if(date == _selectedDay)
      fill = selectedFill;
    else if(date.month() != _selectedDay.month())
    {
      fill = nonMonthFill;
      dayFill = nonMonthDayFill;
    }
    else if(date == today)
      fill = todayFill;
    else if(date.dayOfWeek() > 5)
      fill = weekendFill;

    QString additionalText;
    if(_controller)
      additionalText = _controller->contents(date);

    QGraphicsRectItem * ri = static_cast<QGraphicsRectItem*>(_items[QString("day%1").arg(wday)]);
    if(ri)
      ri->setBrush(fill);
  
    QGraphicsSimpleTextItem * si = static_cast<QGraphicsSimpleTextItem*>(_items[QString("day%1Number").arg(wday)]);
    if(si)
    {
      si->setText(QString::number(date.day()));
      si->setBrush(dayFill);
    }

    QGraphicsSimpleTextItem * si2 = static_cast<QGraphicsSimpleTextItem*>(_items[QString("day%1Text").arg(wday)]);
    if(si2)
    {
      si2->setText(additionalText);
      si2->setBrush(dayFill);
    }
  }
  QApplication::restoreOverrideCursor();

  if(_controller)
   _controller->setSelectedDay(_selectedDay);
}

// protected functions from base class that we don't want other people to change
void CalendarGraphicsItem::setRect(const QRectF & rectangle)
{
  QGraphicsRectItem::setRect(rectangle);
}

void CalendarGraphicsItem::setRect(qreal x, qreal y, qreal width, qreal height)
{
  QGraphicsRectItem::setRect(x, y, width, height);
}

// reimplemented from GraphicsButtonReceiver
void CalendarGraphicsItem::buttonMouseReleaseEvent(QGraphicsItem * origin, QGraphicsSceneMouseEvent * /*event*/)
{
  QDate date = _selectedDay;
  if(origin == _items["fastforward"])
    date = date.addYears(1);
  else if(origin == _items["forward"])
    date = date.addMonths(1);
  else if(origin == _items["rewind"])
    date = date.addMonths(-1);
  else if(origin == _items["fastrewind"])
    date = date.addYears(-1);
  setSelectedDay(date);
}

void CalendarGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
  // good enough to capture the mouse
}

void CalendarGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsItem * item;
  for(int i = 0; i < 42; ++i)
  {
    item = _items[QString("day%1").arg(i)];
    if(item && item->contains(item->mapFromScene(event->scenePos())))
    {
      QDate firstMonthDay = QDate(_selectedDay.year(), _selectedDay.month(), 1);
      QDate firstCalendarDay = firstMonthDay.addDays(firstMonthDay.dayOfWeek() * -1);
      if(firstMonthDay.dayOfWeek() < 2)
        firstCalendarDay = firstCalendarDay.addDays(-7);
      setSelectedDay(firstCalendarDay.addDays(i));
      return;
    }
  }
}

