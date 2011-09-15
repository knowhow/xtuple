/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xmainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QWorkspace>
#include <QStatusBar>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>

#include "xcheckbox.h"
#include "xtsettings.h"
#include "guiclient.h"
#include "scriptablePrivate.h"
#include "shortcuts.h"

//
// XMainWindowPrivate
//
class XMainWindowPrivate : public ScriptablePrivate
{
  friend class XMainWindow;

  public:
    XMainWindowPrivate(XMainWindow*);
    ~XMainWindowPrivate();

    XMainWindow * _parent;
    bool _shown;
    QAction *_action;
};

XMainWindowPrivate::XMainWindowPrivate(XMainWindow * parent) : ScriptablePrivate(false, parent), _parent(parent)
{
  _shown = false;
  _action = 0;
}

XMainWindowPrivate::~XMainWindowPrivate()
{
  if(_action)
    delete _action;
}

XMainWindow::XMainWindow(QWidget * parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
{
  _private = new XMainWindowPrivate(this);

  _private->_action = new QAction(this);
  _private->_action->setShortcutContext(Qt::ApplicationShortcut);
  _private->_action->setText(windowTitle());
  _private->_action->setCheckable(true);
  connect(_private->_action, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
}

XMainWindow::XMainWindow(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
{
  if(name)
    setObjectName(name);

  _private = new XMainWindowPrivate(this);

  _private->_action = new QAction(this);
  _private->_action->setShortcutContext(Qt::ApplicationShortcut);
  _private->_action->setText(windowTitle());
  _private->_action->setCheckable(true);
  connect(_private->_action, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
}

XMainWindow::~XMainWindow()
{
  if(_private)
    delete _private;
}

enum SetResponse XMainWindow::set(const ParameterList &pParams)
{
  _lastSetParams = pParams;

  _private->loadScriptEngine();
  QTimer::singleShot(0, this, SLOT(postSet()));

  return NoError;
}

enum SetResponse XMainWindow::postSet()
{
  return _private->callSet(_lastSetParams);
}

ParameterList XMainWindow::get() const
{
  return _lastSetParams;
}

void XMainWindow::closeEvent(QCloseEvent *event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  _private->callCloseEvent(event);

  if(event->isAccepted())
  {
    QString objName = objectName();
    xtsettingsSetValue(objName + "/geometry/size", size());
    if(omfgThis->showTopLevel() || isModal())
      xtsettingsSetValue(objName + "/geometry/pos", pos());
    else
      xtsettingsSetValue(objName + "/geometry/pos", parentWidget()->pos());
  }
}

void XMainWindow::showEvent(QShowEvent *event)
{
  if(!_private->_shown)
  {
    _private->_shown = true;
//qDebug("isModal() %s", isModal()?"true":"false");

    QRect availableGeometry = QApplication::desktop()->availableGeometry();
    if(!omfgThis->showTopLevel() && !isModal())
      availableGeometry = omfgThis->workspace()->geometry();

    QString objName = objectName();
    QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
    QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();

    if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool() && (metaObject()->className() != QString("xTupleDesigner")))
      resize(lsize);

    setAttribute(Qt::WA_DeleteOnClose);
    if(omfgThis->showTopLevel() || isModal())
    {
      omfgThis->_windowList.append(this);
      statusBar()->show();
      QRect r(pos, size());
      if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
        move(pos);
    }
    else
    {
      QWidget * fw = focusWidget();
      omfgThis->workspace()->addWindow(this);
      QRect r(pos, size());
      if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
        move(pos);
      // This originally had to be after the show? Will it work here?
      if(fw)
        fw->setFocus();
    }

    _private->loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();

    shortcuts::setStandardKeys(this);
  }

  bool blocked = _private->_action->blockSignals(true);
  _private->_action->setChecked(true);
  _private->_action->blockSignals(blocked);

  _private->callShowEvent(event);

  QMainWindow::showEvent(event);
}

void XMainWindow::hideEvent(QHideEvent * event)
{
  bool blocked = _private->_action->blockSignals(true);
  _private->_action->setChecked(false);
  _private->_action->blockSignals(blocked);

  QMainWindow::hideEvent(event);
}

QAction *XMainWindow::action() const
{
  return _private->_action;
}

void XMainWindow::changeEvent(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::WindowTitleChange:
        _private->_action->setText(windowTitle());
        break;
    case QEvent::WindowIconChange:
        _private->_action->setIcon(windowIcon());
        break;
    default:
        break;
  }
  QMainWindow::changeEvent(e);
}

void XMainWindow::showMe(bool v)
{
  QWidget *target = parentWidget() == 0 ? this : parentWidget();

  if (v)
    target->setWindowState(target->windowState() & ~Qt::WindowMinimized);

  target->setVisible(v);
}

QScriptEngine *engine(XMainWindow *win)
{
  if(win && win->_private)
    return win->_private->engine();
  return 0;
}

