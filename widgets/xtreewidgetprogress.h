/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XTREEWIDGETPROGRESS_H
#define XTREEWIDGETPROGRESS_H
#include "xtreewidget.h"

class QLayout;
class QProgressBar;
class QPushButton;

class XTreeWidgetProgress : public QWidget
{
  Q_OBJECT

  public:
    XTreeWidgetProgress(XTreeWidget *parent);

  public slots:
    virtual void setMaximum(int max);
    virtual void setOrientation(Qt::Orientation o);
    virtual void setValue(int val);

  signals:
    void cancel();

  protected:
    virtual void hideEvent(QHideEvent *event);
    virtual void showEvent(QShowEvent *event);

  private:
    QPushButton  *_cancel;
    QLayout      *_lyt;
    QProgressBar *_pb;
};

#endif
