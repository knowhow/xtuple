/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xmessageboxmessagehandler.h"

#include <QMessageBox>

XMessageBoxMessageHandler::XMessageBoxMessageHandler(QObject *p)
  : QAbstractMessageHandler(p)
{
  _message = "";
  _title   = tr("Error");
  _prefix  = tr("<p>There was a problem reading the file, "
                "line %1 column %2: %3<br>(%4)");
}

QString XMessageBoxMessageHandler::lastMessage() const
{
  return _message;
}

void XMessageBoxMessageHandler::setTitle(const QString &title)
{
  _title = title;
}

QString XMessageBoxMessageHandler::title() const
{
  return _title;
}

void XMessageBoxMessageHandler::setPrefix(const QString &prefix)
{
  _prefix = prefix;
}

void XMessageBoxMessageHandler::handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  if (description.isEmpty())
  {
    _message = description;
    return;
  }

  _message = _prefix.arg(sourceLocation.line())
                    .arg(sourceLocation.column())
                    .arg(description)
                    .arg(identifier.toString());
  switch (type)
  {
    case QtDebugMsg:
      QMessageBox::information(0, _title, _message);
      break;
    case QtWarningMsg:
      QMessageBox::warning(0, _title, _message);
      break;
    case QtCriticalMsg:
    case QtFatalMsg:
    default:
      QMessageBox::critical(0, _title, _message);
      break;
  }
}
