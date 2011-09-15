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

class QGridLayout;
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
    
  public slots:
    void sIndexChanged(const QUrl&);
    void sLocationChanged(Qt::DockWidgetArea);

  protected:
    xtHelp              *_help;
    helpViewBrowser     *_helpBrowser;
    QToolBar            *_helpBrowserToolbar;
    
    QString             _loc;

  private:
    helpView(QWidget *parent = 0);
    
    QWidget             *_layoutContainer;
    QGridLayout         *_layout;
};

#endif // __HELPVIEW_H__
