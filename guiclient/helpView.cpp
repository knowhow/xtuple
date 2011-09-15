/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QGridLayout>
#include <QHelpContentWidget>
#include <QPixmap>
#include <QToolBar>
#include <QUrl>
#include <QWidget>

#include <QDebug>

#include <quuencode.h>

#include "guiclient.h"
#include "helpView.h"
#include "helpViewBrowser.h"
#include "xtHelp.h"

static QIcon iconFromImageByName(QString name)
{
  QImage    image;
  XSqlQuery imageq;
  imageq.prepare("SELECT image_data FROM image WHERE image_name=:name LIMIT 1;");
  imageq.bindValue(":name", name);
  if (imageq.exec() && imageq.first())
  {
    image.loadFromData(QUUDecode(imageq.value("image_data").toString()));
    return QIcon(QPixmap::fromImage(image));
  }
  return QIcon();
}

static helpView *helpViewSingleton = 0;

helpView* helpView::getInstance(QWidget *parent)
{
  if(!helpViewSingleton)
    helpViewSingleton = new helpView(parent);
  return helpViewSingleton;
}

helpView::helpView(QWidget *parent)
  : QDockWidget(tr("xTuple Help Documentation"), parent)
{
  setObjectName("helpView");

  _layoutContainer = new QWidget(this);
  _layout = new QGridLayout;
  _layout->setSpacing(0);
  _helpBrowser = new helpViewBrowser(this);
  _help = xtHelp::getInstance(this);
  _helpBrowserToolbar = new QToolBar(this);

  QAction *back = _helpBrowserToolbar->addAction(iconFromImageByName("ImgLeftArrow"), tr("Back"),    _helpBrowser, SLOT(backward()));
  QAction *fwd  = _helpBrowserToolbar->addAction(iconFromImageByName("ImgRightArrow"),tr("Forward"), _helpBrowser, SLOT(forward()));
  QAction *home = _helpBrowserToolbar->addAction(iconFromImageByName("home_32"),      tr("Home"),    _helpBrowser, SLOT(home()));
  connect(_helpBrowser, SIGNAL(backwardAvailable(bool)), back, SLOT(setEnabled(bool)));
  connect(_helpBrowser, SIGNAL(forwardAvailable(bool)),  fwd,  SLOT(setEnabled(bool)));
  connect(_helpBrowser, SIGNAL(backwardAvailable(bool)), home, SLOT(setEnabled(bool)));

  connect(_help->contentWidget(),       SIGNAL(linkActivated(const QUrl&)),                     this,   SLOT(sIndexChanged(const QUrl&)));
  connect(this,                         SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),        this,   SLOT(sLocationChanged(Qt::DockWidgetArea)));

  _layout->addWidget(_helpBrowserToolbar,    0, 0, 1, -1);
  _layout->addWidget(_help->contentWidget(), 1, 0);
  if(_help->isOnline())
  {
    _help->contentWidget()->hide();
    sIndexChanged(QUrl("index.html"));
  }
  _layout->addWidget(_helpBrowser, 2, 0);
  _layoutContainer->setLayout(_layout);

  setWidget(_layoutContainer);

  omfgThis->addDockWidget(Qt::TopDockWidgetArea, this);
}

helpView::~helpView()
{
}

void helpView::sIndexChanged(const QUrl& index)
{
  _helpBrowser->setSource(index);
  _helpBrowser->show();
}

void helpView::sLocationChanged(Qt::DockWidgetArea area)
{
  bool redraw = false;
  if(
      area == Qt::RightDockWidgetArea ||
      area == Qt::LeftDockWidgetArea
    )
  {
    _layout->removeWidget(_helpBrowser);
    _layout->addWidget(_helpBrowser,2,0);
    redraw = true;
  }
  else if(
      area == Qt::TopDockWidgetArea ||
      area == Qt::BottomDockWidgetArea
    )
  {
    _layout->removeWidget(_helpBrowser);
    _layout->addWidget(_helpBrowser,1,1);
    redraw = true;
  }
  if(redraw)
  {
    show();
  }
}
