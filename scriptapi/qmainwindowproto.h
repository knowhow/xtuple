/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QMAINWINDOWPROTO_H__
#define __QMAINWINDOWPROTO_H__

#include <QObject>
#include <QtScript>
#include <QByteArray>
#include <QString>
#include <QMainWindow>

Q_DECLARE_METATYPE(QMainWindow*)

void setupQMainWindowProto(QScriptEngine *engine);
void DockOptionScriptValue(const QScriptValue &obj, enum QMainWindow::DockOption &p);

QScriptValue constructQMainWindow(QScriptContext *context, QScriptEngine *engine);
QScriptValue DockOptiontoScriptValue(QScriptEngine *engine, const enum QMainWindow::DockOption &p);

class QMainWindowProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QMainWindowProto(QObject *parent);

    Q_INVOKABLE void	addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget );
    Q_INVOKABLE void	addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget, Qt::Orientation orientation );
    Q_INVOKABLE void	addToolBar ( Qt::ToolBarArea area, QToolBar * toolbar );
    Q_INVOKABLE void	addToolBar ( QToolBar * toolbar );
    Q_INVOKABLE QToolBar *	addToolBar ( const QString & title );
    Q_INVOKABLE void	addToolBarBreak ( Qt::ToolBarArea area = Qt::TopToolBarArea );
    Q_INVOKABLE QWidget *	centralWidget () const;
    Q_INVOKABLE Qt::DockWidgetArea	corner ( Qt::Corner corner ) const;
    Q_INVOKABLE QMenu *	createPopupMenu ();
    Q_INVOKABLE Qt::DockWidgetArea	dockWidgetArea ( QDockWidget * dockwidget ) const;
    Q_INVOKABLE void	insertToolBar ( QToolBar * before, QToolBar * toolbar );
    Q_INVOKABLE void	insertToolBarBreak ( QToolBar * before );
    Q_INVOKABLE QMenuBar *	menuBar () const;
    Q_INVOKABLE QWidget *	menuWidget () const;
    Q_INVOKABLE void	removeDockWidget ( QDockWidget * dockwidget );
    Q_INVOKABLE void	removeToolBar ( QToolBar * toolbar );
    Q_INVOKABLE void	removeToolBarBreak ( QToolBar * before );
    Q_INVOKABLE bool	restoreDockWidget ( QDockWidget * dockwidget );
    Q_INVOKABLE bool	restoreState ( int version = 0 );
    Q_INVOKABLE void	saveState ( int version = 0 ) const;
    Q_INVOKABLE void	setCentralWidget ( QWidget * widget );
    Q_INVOKABLE void	setCorner ( Qt::Corner corner, Qt::DockWidgetArea area );
    Q_INVOKABLE void	setMenuBar ( QMenuBar * menuBar );
    Q_INVOKABLE void	setMenuWidget ( QWidget * menuBar );
    Q_INVOKABLE void	setStatusBar ( QStatusBar * statusbar );
    Q_INVOKABLE void	setTabPosition ( Qt::DockWidgetAreas areas, QTabWidget::TabPosition tabPosition );
    Q_INVOKABLE void	splitDockWidget ( QDockWidget * first, QDockWidget * second, Qt::Orientation orientation );
    Q_INVOKABLE QStatusBar *	statusBar () const;
    Q_INVOKABLE QTabWidget::TabPosition	tabPosition ( Qt::DockWidgetArea area ) const;
    Q_INVOKABLE QList<QDockWidget *>	tabifiedDockWidgets ( QDockWidget * dockwidget ) const;
    Q_INVOKABLE void	tabifyDockWidget ( QDockWidget * first, QDockWidget * second );
    Q_INVOKABLE Qt::ToolBarArea	toolBarArea ( QToolBar * toolbar ) const;
    Q_INVOKABLE bool	toolBarBreak ( QToolBar * toolbar ) const;
};

#endif
