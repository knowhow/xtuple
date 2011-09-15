/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <stdio.h>

#include "errorLog.h"
#include "guiclient.h"

#include <QObject>
#include <QVariant>
#include <QMessageBox>
#include <QStringList>
#include <QDateTime>
#include <QSqlError>

#include "xtsettings.h"

static QStringList _errorList;
static errorLogListener * listener = 0;

void errorLogListener::initialize()
{
  listener = new errorLogListener();
}

void errorLogListener::destroy()
{
  if(listener)
    delete listener;
  listener = 0;
}

errorLog::errorLog(QWidget* parent, const char * name, Qt::WFlags flags)
    : XWidget(parent, name, flags)
{
  setupUi(this);

  for(int i = 0; i < _errorList.size(); i++)
    _errorLog->append(_errorList.at(i));

  _debug->setChecked(xtsettingsValue("catchQDebug").toBool());
  _warning->setChecked(xtsettingsValue("catchQWarning").toBool());
  _critical->setChecked(xtsettingsValue("catchQCritical").toBool());
  _fatal->setChecked(xtsettingsValue("catchQFatal").toBool());

  connect(_clear,   SIGNAL(clicked()),           _errorLog, SLOT(clear()));
  connect(_clear,   SIGNAL(clicked()),            listener, SLOT(clear()));
  connect(_clear,   SIGNAL(clicked()),            omfgThis, SLOT(sClearErrorMessages()));
  connect(listener, SIGNAL(updated(const QString &)), this, SLOT(updateErrors(const QString &)));
  connect(_debug,   SIGNAL(toggled(bool)),        this,     SLOT(toggleDebug(bool)));
  connect(_warning, SIGNAL(toggled(bool)),        this,     SLOT(toggleWarning(bool)));
  connect(_critical,SIGNAL(toggled(bool)),        this,     SLOT(toggleCritical(bool)));
  connect(_fatal,   SIGNAL(toggled(bool)),        this,     SLOT(toggleFatal(bool)));
}

errorLog::~errorLog()
{
  // no need to delete child widgets, Qt does it all for us
}

void errorLog::languageChange()
{
  retranslateUi(this);
}

void errorLog::updateErrors(const QString & msg)
{
  _errorLog->append(msg);
}

void errorLog::toggleDebug(bool y)
{
  xtsettingsSetValue("catchQDebug", y);
}

void errorLog::toggleWarning(bool y)
{
  xtsettingsSetValue("catchQWarning", y);
}

void errorLog::toggleCritical(bool y)
{
  xtsettingsSetValue("catchQCritical", y);
}

void errorLog::toggleFatal(bool y)
{
  xtsettingsSetValue("catchQFatal", y);
}

errorLogListener::errorLogListener(QObject * parent)
  : QObject(parent)
{
  XSqlQuery::addErrorListener(this);
}

errorLogListener::~errorLogListener()
{
  XSqlQuery::removeErrorListener(this);
}

void errorLogListener::error(const QString & sql, const QSqlError & error)
{
  QString msg;
  msg = QDateTime::currentDateTime().toString();
  msg += " " + error.text();
  msg += "\n" + sql;

  _errorList.append(msg);
  if(_errorList.size() > 20)
    _errorList.removeFirst();

  emit updated(msg);
  if(omfgThis)
    omfgThis->sNewErrorMessage();
}

void errorLogListener::clear()
{
  bool blocked = blockSignals(true);
  _errorList.clear();
  (void)blockSignals(blocked);
}

void xTupleMessageOutput(QtMsgType type, const char *pMsg)
{
  QString msg;
  msg = QDateTime::currentDateTime().toString();
  bool notify = false;
  bool iserror= (type == QtCriticalMsg) || (type == QtFatalMsg);

  switch (type) {
    case QtDebugMsg:
      notify = xtsettingsValue("catchQDebug").toBool();
      msg += " Debug: ";
      break;
    case QtWarningMsg:
      notify = xtsettingsValue("catchQWarning").toBool();
      msg += " Warning: ";
      break;
    case QtCriticalMsg:
      notify = xtsettingsValue("catchQCritical").toBool();
      msg += " Critical: ";
      break;
    case QtFatalMsg:
      notify = xtsettingsValue("catchQFatal").toBool();
      msg += " Fatal: ";
      //abort();
      break;
  }
  msg += pMsg;
  _errorList.append(msg);
  if(_errorList.size() > 20)
    _errorList.removeFirst();

  if(listener && notify)
    listener->updated(msg);
  if(omfgThis && notify && iserror)
    omfgThis->sNewErrorMessage();

  printf("%s\n", qPrintable(msg));
}

