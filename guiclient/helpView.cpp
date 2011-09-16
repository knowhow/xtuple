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
#include <QHelpSearchEngine>
#include <QHelpSearchQueryWidget>
#include <QHelpSearchResultWidget>
#include <QHelpIndexWidget>
#include <QPixmap>
#include <QSplitter>
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

void helpView::reset()
{
  helpViewSingleton = 0;
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
  _searchSplitter = new QSplitter(this);
  _mainSplitter = new QSplitter(this);
  _searchEngine = new QHelpSearchEngine(_help, this);
  _searchTabs = new QTabWidget(this);

  QAction *back = _helpBrowserToolbar->addAction(iconFromImageByName("ImgLeftArrow"), tr("Back"),    _helpBrowser, SLOT(backward()));
  QAction *fwd  = _helpBrowserToolbar->addAction(iconFromImageByName("ImgRightArrow"),tr("Forward"), _helpBrowser, SLOT(forward()));
  QAction *home = _helpBrowserToolbar->addAction(iconFromImageByName("home_32"),      tr("Home"),    _helpBrowser, SLOT(home()));
  connect(_helpBrowser, SIGNAL(backwardAvailable(bool)), back, SLOT(setEnabled(bool)));
  connect(_helpBrowser, SIGNAL(forwardAvailable(bool)),  fwd,  SLOT(setEnabled(bool)));
  connect(_helpBrowser, SIGNAL(backwardAvailable(bool)), home, SLOT(setEnabled(bool)));

  connect(_searchEngine->queryWidget(),SIGNAL(search()), this, SLOT(queriesToEngine()));

  connect(_help->contentWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(showLink(QModelIndex)));
  connect(_help->contentWidget(),       SIGNAL(linkActivated(const QUrl&)),                     this,   SLOT(sIndexChanged(const QUrl&)));
  connect(_searchEngine->resultWidget(), SIGNAL(requestShowLink(const QUrl&)), this, SLOT(sIndexChanged(const QUrl&)));
  connect(_help->indexWidget(), SIGNAL(linkActivated(const QUrl&,QString)), this, SLOT(sIndexChanged(const QUrl&)));
  connect(this,                         SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),        this,   SLOT(sLocationChanged(Qt::DockWidgetArea)));

  _searchSplitter->setOrientation(Qt::Vertical);
  _searchSplitter->addWidget(_searchEngine->queryWidget());
  _searchSplitter->addWidget(_searchEngine->resultWidget());

  _searchTabs->addTab(_help->contentWidget(), "Contents");
  _searchTabs->addTab(_searchSplitter, "Search");
  _searchTabs->addTab(_help->indexWidget(), "Index");

  _mainSplitter->addWidget(_searchTabs);
  _mainSplitter->addWidget(_helpBrowser);

  _layout->addWidget(_helpBrowserToolbar,    0, 0, 1, -1);
  _layout->addWidget(_mainSplitter, 1, 0);

  if(_help->isOnline())
  {
    _help->contentWidget()->hide();
    sIndexChanged(QUrl("index.html"));
  }
  _layoutContainer->setLayout(_layout);

  setWidget(_layoutContainer);

  omfgThis->addDockWidget(Qt::TopDockWidgetArea, this);

  _searchEngine->reindexDocumentation();

}

helpView::~helpView()
{
}

void helpView::queriesToEngine()
{
    _searchEngine->search(_searchEngine->queryWidget()->query());

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
    _mainSplitter->addWidget(_helpBrowser);
    redraw = true;
  }
  else if(
      area == Qt::TopDockWidgetArea ||
      area == Qt::BottomDockWidgetArea
    )
  {
    _layout->removeWidget(_helpBrowser);
    _mainSplitter->addWidget(_helpBrowser);
    redraw = true;
  }
  if(redraw)
  {
    show();
  }
}

void helpView::showLink(const QModelIndex &index)
{
    QHelpContentModel *contentModel = _help->contentModel();
    if (!contentModel)
        return;

    QHelpContentItem *item = contentModel->contentItemAt(index);
    if (!item)
        return;
    QUrl url = item->url();
    if (url.isValid())
        sIndexChanged(url);
}
