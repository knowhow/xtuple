/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __HELPVIEW_H__
#define __HELPVIEW_H__

#include <QDockWidget>
#include <QModelIndex>

class QGridLayout;
class QHelpSearchEngine;
class QTabWidget;
class QSplitter;
class QToolBar;
class QUrl;
class QWidget;

class xtHelp;
class helpViewBrowser;

class helpView : public QDockWidget
{
  Q_OBJECT
  public:
    static helpView     *getInstance(QWidget *parent = 0);
    ~helpView();
    
    static void reset();

  public slots:
    void queriesToEngine();
    void sIndexChanged(const QUrl&);
    void sLocationChanged(Qt::DockWidgetArea);
    void showLink(const QModelIndex &index);

  protected:
    xtHelp              *_help;
    helpViewBrowser     *_helpBrowser;
    QTabWidget          *_searchTabs;
    QToolBar            *_helpBrowserToolbar;
    QSplitter           *_searchSplitter;
    QSplitter           *_mainSplitter;
    QHelpSearchEngine   *_searchEngine;


    QString             _loc;

  private:
    helpView(QWidget *parent = 0);
    
    QWidget             *_layoutContainer;
    QGridLayout         *_layout;
};

#endif // __HELPVIEW_H__
