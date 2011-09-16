/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>

#include "xcheckbox.h"
#include "xtsettings.h"
#include "guiclient.h"
#include "scriptablePrivate.h"
#include "shortcuts.h"

//
// XDialogPrivate
//
class XDialogPrivate : public ScriptablePrivate
{
  friend class XDialog;

  public:
    XDialogPrivate(XDialog*);
    ~XDialogPrivate();

    XDialog * _parent;
    bool _shown;
    QAction *_rememberPos;
    QAction *_rememberSize;
    ParameterList lastSetParams;
};

XDialogPrivate::XDialogPrivate(XDialog *parent)
  : ScriptablePrivate(true, parent), _parent(parent)
{
  _shown = false;
  _rememberPos = 0;
  _rememberSize = 0;
}

XDialogPrivate::~XDialogPrivate()
{
  if(_rememberPos)
    delete _rememberPos;
  if(_rememberSize)
    delete _rememberSize;
}

XDialog::XDialog(QWidget * parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  connect(this, SIGNAL(destroyed(QObject*)), omfgThis, SLOT(windowDestroyed(QObject*)));
  connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));
  _private = new XDialogPrivate(this);
}

XDialog::XDialog(QWidget * parent, const char * name, bool modal, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  if(name)
    setObjectName(name);
  if(modal)
    setModal(modal);

  connect(this, SIGNAL(destroyed(QObject*)), omfgThis, SLOT(windowDestroyed(QObject*)));
  connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));

  _private = new XDialogPrivate(this);
}

XDialog::~XDialog()
{
  if(_private)
    delete _private;
}

void XDialog::saveSize()
{
  xtsettingsSetValue(objectName() + "/geometry/size", size());
  xtsettingsSetValue(objectName() + "/geometry/pos", pos());
}

void XDialog::closeEvent(QCloseEvent * event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  _private->callCloseEvent(event);

  if(event->isAccepted())
  {
    saveSize();
    QDialog::closeEvent(event);
  }
}

void XDialog::showEvent(QShowEvent *event)
{
  if(!_private->_shown)
  {
    _private->_shown = true;

    QRect availableGeometry = QApplication::desktop()->availableGeometry();

    QString objName = objectName();
    QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
    QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();

    if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
      resize(lsize);

    // do I want to do this for a dialog?
    //_windowList.append(w);
    QRect r(pos, size());
    if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
      move(pos);

    _private->_rememberPos = new QAction(tr("Remember Position"), this);
    _private->_rememberPos->setCheckable(true);
    _private->_rememberPos->setChecked(xtsettingsValue(objectName() + "/geometry/rememberPos", true).toBool());
    connect(_private->_rememberPos, SIGNAL(triggered(bool)), this, SLOT(setRememberPos(bool)));
    _private->_rememberSize = new QAction(tr("Remember Size"), this);
    _private->_rememberSize->setCheckable(true);
    _private->_rememberSize->setChecked(xtsettingsValue(objectName() + "/geometry/rememberSize", true).toBool());
    connect(_private->_rememberSize, SIGNAL(triggered(bool)), this, SLOT(setRememberSize(bool)));

    addAction(_private->_rememberPos);
    addAction(_private->_rememberSize);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    _private->loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();

    shortcuts::setStandardKeys(this);
  }

  _private->callShowEvent(event);

  QDialog::showEvent(event);
}

void XDialog::setRememberPos(bool b)
{
  xtsettingsSetValue(objectName() + "/geometry/rememberPos", b);
  if(_private && _private->_rememberPos)
    _private->_rememberPos->setChecked(b);
}

void XDialog::setRememberSize(bool b)
{
  xtsettingsSetValue(objectName() + "/geometry/rememberSize", b);
  if(_private && _private->_rememberSize)
    _private->_rememberSize->setChecked(b);
}

enum SetResponse XDialog::set(const ParameterList & pParams)
{
  _lastSetParams = pParams;

  _private->loadScriptEngine();

  QTimer::singleShot(0, this, SLOT(postSet()));

  return NoError;
}

enum SetResponse XDialog::postSet()
{
  return _private->callSet(_lastSetParams);
}

ParameterList XDialog::get() const
{
  return _lastSetParams;
}

