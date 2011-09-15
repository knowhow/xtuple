/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "graphicstextbuttonitem.h"

#include <QGraphicsSceneMouseEvent>

#include <QDebug>

GraphicsTextButtonItem::GraphicsTextButtonItem(QGraphicsItem * parent)
  : QGraphicsSimpleTextItem(parent)
{
  setAcceptsHoverEvents(true);
  _receiver = 0;
}

GraphicsTextButtonItem::GraphicsTextButtonItem(const QString & text, QGraphicsItem * parent)
  : QGraphicsSimpleTextItem(text, parent)
{
  setAcceptsHoverEvents(true);
  _receiver = 0;
}

GraphicsTextButtonItem::~GraphicsTextButtonItem()
{
}

void GraphicsTextButtonItem::setRolloverBrush(const QBrush & rollover)
{
  _rolloverBrush = rollover;
}

QBrush GraphicsTextButtonItem::rolloverBrush() const
{
  return _rolloverBrush;
}

void GraphicsTextButtonItem::setReceiver(GraphicsButtonReceiver * receiver)
{
  _receiver = receiver;
}

void GraphicsTextButtonItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
  _normalBrush = brush();
  setBrush(_rolloverBrush);
  QGraphicsSimpleTextItem::hoverEnterEvent(event);
}

void GraphicsTextButtonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  setBrush(_normalBrush);
  QGraphicsSimpleTextItem::hoverLeaveEvent(event);
}

void GraphicsTextButtonItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  if(_receiver)
    _receiver->buttonMouseDoubleClickEvent(this, event);
  //QGraphicsSimpleTextItem::mouseDoubleClickEvent(event);
}

void GraphicsTextButtonItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  if(_receiver)
    _receiver->buttonMousePressEvent(this, event);
  //QGraphicsSimpleTextItem::mousePressEvent(event);
}

void GraphicsTextButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  if(_receiver)
    _receiver->buttonMouseReleaseEvent(this, event);
  //QGraphicsSimpleTextItem::mouseReleaseEvent(event);
}

