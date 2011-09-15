/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qmainwindowproto.h"
#include "qtsetup.h"
#include "xtsettings.h"
#include "qtabwidgetproto.h"

QScriptValue QMainWindowtoScriptValue(QScriptEngine *engine, QMainWindow* const &item)
{
  return engine->newQObject(item);
}

void QMainWindowfromScriptValue(const QScriptValue &obj, QMainWindow* &item)
{
  item = qobject_cast<QMainWindow*>(obj.toQObject());
}

void setupQMainWindowProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QMainWindowtoScriptValue, QMainWindowfromScriptValue);

  QScriptValue proto = engine->newQObject(new QMainWindowProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QMainWindow*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQMainWindow,
                                                 proto);
  engine->globalObject().setProperty("QMainWindow",  constructor);
}

QScriptValue constructQMainWindow(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QMainWindow *obj = 0;
  if (context->argumentCount() == 2)
  {
    obj = new QMainWindow(qobject_cast<QWidget*>(context->argument(0).toQObject()),
                          (Qt::WindowFlags)(context->argument(1).toInt32()));
  }
  if (context->argumentCount() == 1)
    obj = new QMainWindow(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QMainWindow();
  return engine->toScriptValue(obj);
}

QMainWindowProto::QMainWindowProto(QObject *parent)
    : QObject(parent)
{
}

void QMainWindowProto::addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->addDockWidget(area, dockwidget);
}

void QMainWindowProto::addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget, Qt::Orientation orientation )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->addDockWidget(area, dockwidget, orientation);
}

void QMainWindowProto::addToolBar ( Qt::ToolBarArea area, QToolBar * toolbar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->addToolBar(area, toolbar);
}

void QMainWindowProto::addToolBar ( QToolBar * toolbar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->addToolBar(toolbar);
}

QToolBar * QMainWindowProto::addToolBar ( const QString & title )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->addToolBar(title);
  return 0;
}

void QMainWindowProto::addToolBarBreak ( Qt::ToolBarArea area )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->addToolBarBreak(area);
}

QWidget* QMainWindowProto::centralWidget () const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->centralWidget();
  return 0;
}

Qt::DockWidgetArea QMainWindowProto::corner ( Qt::Corner corner ) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->corner(corner);
  return Qt::NoDockWidgetArea;
}

QMenu* QMainWindowProto::createPopupMenu ()
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->createPopupMenu();
  return 0;
}

Qt::DockWidgetArea QMainWindowProto::dockWidgetArea ( QDockWidget * dockwidget ) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->dockWidgetArea (dockwidget);
  return Qt::NoDockWidgetArea;
}

void QMainWindowProto::insertToolBar ( QToolBar * before, QToolBar * toolbar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->insertToolBar(before, toolbar);
}

void QMainWindowProto::insertToolBarBreak ( QToolBar * before )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->insertToolBarBreak(before);
}

QMenuBar* QMainWindowProto::menuBar () const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->menuBar();
  return 0;
}

QWidget* QMainWindowProto::menuWidget () const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->menuWidget();
  return 0;
}

void QMainWindowProto::removeDockWidget ( QDockWidget * dockwidget )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->removeDockWidget(dockwidget);
}

void QMainWindowProto::removeToolBar ( QToolBar * toolbar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->removeToolBar(toolbar);
}

void QMainWindowProto::removeToolBarBreak ( QToolBar * before )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->removeToolBarBreak (before);
}

bool QMainWindowProto::restoreDockWidget ( QDockWidget * dockwidget )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->restoreDockWidget(dockwidget);
  return false;
}

bool QMainWindowProto::restoreState ( int version )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
  {
    QByteArray state;
    state = xtsettingsValue(item->objectName() + "/WindowState", QByteArray()).toByteArray();
    return item->restoreState(state, version);
  }
  return false;
}

void QMainWindowProto::saveState ( int version) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
  {
    QByteArray state = item->saveState(version);
    xtsettingsSetValue(item->objectName() + "/WindowState", state);
  }
}

void QMainWindowProto::setCentralWidget ( QWidget * widget )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->setCentralWidget(widget);
}

void QMainWindowProto::setCorner ( Qt::Corner corner, Qt::DockWidgetArea area )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->setCorner(corner, area);
}

void QMainWindowProto::setMenuBar ( QMenuBar * menuBar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->setMenuBar(menuBar);
}

void QMainWindowProto::setMenuWidget ( QWidget * menuBar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->setMenuWidget(menuBar);
}

void QMainWindowProto::setStatusBar ( QStatusBar * statusbar )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->setStatusBar(statusbar);
}

void QMainWindowProto::setTabPosition ( Qt::DockWidgetAreas areas, QTabWidget::TabPosition tabPosition )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->setTabPosition (areas, tabPosition);
}

void QMainWindowProto::splitDockWidget ( QDockWidget * first, QDockWidget * second, Qt::Orientation orientation )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->splitDockWidget (first, second, orientation);
}

QStatusBar* QMainWindowProto::statusBar () const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->statusBar();
  return 0;
}

QTabWidget::TabPosition QMainWindowProto::tabPosition ( Qt::DockWidgetArea area ) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->tabPosition (area);
  return QTabWidget::North;
}

QList<QDockWidget *> QMainWindowProto::tabifiedDockWidgets ( QDockWidget * dockwidget ) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->tabifiedDockWidgets (dockwidget);
  return QList<QDockWidget *>();
}

void QMainWindowProto::tabifyDockWidget ( QDockWidget * first, QDockWidget * second )
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    item->tabifyDockWidget (first, second);
}

Qt::ToolBarArea QMainWindowProto::toolBarArea ( QToolBar * toolbar ) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return item->toolBarArea(toolbar);
  return Qt::NoToolBarArea;
}

bool QMainWindowProto::toolBarBreak ( QToolBar * toolbar ) const
{
  QMainWindow *item = qscriptvalue_cast<QMainWindow*>(thisObject());
  if (item)
    return toolBarBreak (toolbar);
  return false;
}



