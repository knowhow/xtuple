/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtreewidgetprogress.h"

#include <QBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include "xtreewidget.h"

#define DEBUG false

XTreeWidgetProgress::XTreeWidgetProgress(XTreeWidget *parent)
      : QWidget(parent),
        _cancel(0),
        _lyt(0),
        _pb(0)
{
  setObjectName("XTreeWidgetProgress");

  if (DEBUG)
    qDebug("%s::XTreeWidgetProgress(%p (%s))",
           qPrintable(objectName()), parent,
           qPrintable(parent->objectName()));
  if (parent)
  {
    _pb = new QProgressBar(this);
    _pb->setMinimum(0);

    _cancel = new QPushButton(tr("Stop"), this);

    connect(_cancel, SIGNAL(clicked()), this,    SIGNAL(cancel()));
    connect(_cancel, SIGNAL(clicked()), this,    SLOT(hide()));

    _lyt = new QHBoxLayout();
    _lyt->addWidget(_pb);
    _lyt->addWidget(_cancel);

    QLayout *parentlyt =  0;
    int lytindex       = -1;
    for (QObject *ancestor = parent; ancestor && ! parentlyt;
         ancestor = ancestor->parent())
    {
      QList<QLayout*> list = ancestor->findChildren<QLayout*>();
      for (int i = 0; i < list.size(); i++)
      {
        lytindex = list.at(i)->indexOf(parent);
        if (lytindex != -1)
        {
          if (DEBUG)
            qDebug("%s::XTreeWidgetProgress() found parent layout",
                   qPrintable(objectName()));
          parentlyt = list.at(i);
          break;
        }
      }
    }
    if (parentlyt)
    {
      if (QGridLayout *g = qobject_cast<QGridLayout*>(parentlyt))
      {
        int row = 0;
        int col = 0;
        int rowspan = 0;
        int colspan = 0;
        g->getItemPosition(lytindex, &row, &col, &rowspan, &colspan);
        g->addLayout(_lyt, row + 1, col, rowspan, colspan);
        if (DEBUG)
          qDebug("%s::XTreeWidgetProgress() inserted into grid layout",
                 qPrintable(objectName()));
      }
      else if (QBoxLayout *b = qobject_cast<QBoxLayout*>(parentlyt))
      {
        b->insertLayout(lytindex + 1, _lyt);
        _pb->setOrientation(qobject_cast<QVBoxLayout*>(parentlyt) ?
                                            Qt::Horizontal : Qt::Vertical);
        if (DEBUG)
          qDebug("%s::XTreeWidgetProgress() inserted into box layout (%d)",
                 qPrintable(objectName()), _pb->orientation());
      }
    }
    else if (DEBUG)
      qDebug("%s::XTreeWidgetProgress() could not find parent layout",
             qPrintable(objectName()));
  }
}

void XTreeWidgetProgress::setMaximum(int max)
{
  _pb->setMaximum(max);
}

void XTreeWidgetProgress::setOrientation(Qt::Orientation o)
{
  if (DEBUG)
    qDebug("%s::setOrientation(%d)", qPrintable(objectName()), o);
  _pb->setOrientation(o);
}

void XTreeWidgetProgress::setValue(int val)
{
  if (DEBUG)
    qDebug("%s::setValue(%d)", qPrintable(objectName()), val);
  _pb->setValue(val);
}

void XTreeWidgetProgress::hideEvent(QHideEvent *event)
{
  _cancel->hide();
  _pb->hide();
  QWidget::hideEvent(event);
}

void XTreeWidgetProgress::showEvent(QShowEvent *event)
{
  _cancel->show();
  _pb->show();
  QWidget::showEvent(event);
}
