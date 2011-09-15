/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xurllabel.h"
#include <QMouseEvent>
#include <QLabel>

XURLLabel::XURLLabel(QWidget * parent, const char * name)
  : QLabel(parent)
{
  setObjectName(name ? name : "XUrlLabel");
  QPalette p = palette();
  p.setColor(QPalette::Foreground, Qt::blue);
  setPalette(p);

  setFont(font());
}

XURLLabel::~XURLLabel() {}

void XURLLabel::mouseReleaseEvent(QMouseEvent * e)
{
  QLabel::mouseReleaseEvent(e);

  switch(e->button())
  {
    case Qt::LeftButton:
      e->accept();
      emit leftClickedURL(_url);
      break;
    case Qt::MidButton:
      e->accept();
      emit middleClickedURL(_url);
      break;
    case Qt::RightButton:
      e->accept();
      emit rightClickedURL(_url);
      break;
    default:
      e->ignore();
      break;
  }
}

void XURLLabel::setFont(const QFont & f)
{
  QFont newFont = f;
  newFont.setUnderline(TRUE);

  QLabel::setFont(newFont);
}

void XURLLabel::setURL(const QString & url)
{
  _url = url;
}

const QString & XURLLabel::url() const
{
  return _url;
}

