/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __GRAPHICSTEXTBUTTONITEM_H__
#define __GRAPHICSTEXTBUTTONITEM_H__

#include <QGraphicsSimpleTextItem>

#include <QBrush>

class GraphicsButtonReceiver
{
  public:
    virtual ~GraphicsButtonReceiver() {};
    virtual void buttonMouseDoubleClickEvent(QGraphicsItem * /*origin*/, QGraphicsSceneMouseEvent * /*event*/) {}
    virtual void buttonMousePressEvent(QGraphicsItem * /*origin*/, QGraphicsSceneMouseEvent * /*event*/) {}
    virtual void buttonMouseReleaseEvent(QGraphicsItem * /*origin*/, QGraphicsSceneMouseEvent * /*event*/) {}
};

class GraphicsTextButtonItem : public QGraphicsSimpleTextItem
{
  public:
    GraphicsTextButtonItem(QGraphicsItem * parent = 0);
    GraphicsTextButtonItem(const QString & text, QGraphicsItem * parent = 0);
    ~GraphicsTextButtonItem();

    void setRolloverBrush(const QBrush & rollover);
    QBrush rolloverBrush() const;

    void setReceiver(GraphicsButtonReceiver * receiver);
    
  protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

    QBrush _normalBrush;
    QBrush _rolloverBrush;

    GraphicsButtonReceiver * _receiver;
};

#endif
