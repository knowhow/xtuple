/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QCoreApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QDebug>

#include "guiclient.h"
#include "xtHelp.h"
#include "helpView.h"

static QString QHC_PATH;

static const QString WEBHOMEPAGE = "http://www.xtuple.org/sites/default/files/refguide/RefGuide-%1/index.html";
static const QString QHCHOMEPAGE = "qthelp://xtuple.org/postbooks/index.html";

#define DEBUG false

static xtHelp *xtHelpSingleton = 0;

xtHelp* xtHelp::getInstance(QWidget *parent)
{
  if(QHC_PATH.isEmpty())
  {
    QFile hf(QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/XTupleGUIClient.qhc");
    if(hf.exists())
      QHC_PATH = hf.fileName();
    else
      QHC_PATH = QCoreApplication::instance()->applicationDirPath() +
                    QDir::separator() +
#ifdef Q_OS_MAC
                    "../Resources/" +
#endif
                    "XTupleGUIClient.qhc";
  }
  if(!xtHelpSingleton)
    xtHelpSingleton = new xtHelp(parent);
  return xtHelpSingleton;
}

void xtHelp::reload()
{
  helpView::reset();
  QHC_PATH = "";
  xtHelpSingleton = 0;
}

xtHelp::xtHelp(QWidget *parent)
  : QHelpEngine(QHC_PATH, parent),
  _nam(new QNetworkAccessManager),
  _online(false)
{
  connect(_nam, SIGNAL(finished(QNetworkReply *)), this, SLOT(sError(QNetworkReply *)));

  if(!setupData())
    qWarning("Error setting up the help data");

/* TODO: remove the #ifdef and the empty help file & its corresponding qhc
         from xtuple/share once we fix the remote image loading bug in fileData
*/
#ifdef XTHELPONLINE
  _online = fileData(QUrl(QHCHOMEPAGE)) == QByteArray("");
#endif // XTHELPONLINE

  if (DEBUG)
    qDebug() << "xtHelp collection file" << collectionFile()
             << "exists?"   << QFile::exists(collectionFile())
             << "online?"   << _online;
}

xtHelp::~xtHelp()
{
} 

QUrl xtHelp::homePage() const
{
  if (_online)
    return QUrl(WEBHOMEPAGE.arg(qApp->applicationVersion().left(3)));
  else
    return QUrl(QHCHOMEPAGE);
}

bool xtHelp::isOnline() const
{
  return _online;
}

QByteArray xtHelp::fileData(const QUrl &url) const
{
  QByteArray returnVal;
  if (isOnline() || url.scheme() == "http")
  {
    QNetworkRequest req(url);
    QNetworkReply  *rep = _nam->get(req);
    while(! rep->isFinished())
      QCoreApplication::instance()->processEvents();
    returnVal = rep->readAll();
  }
  else
    returnVal = QHelpEngine::fileData(url);

  if (DEBUG)
    qDebug() << "xtHelp::fileData(" << url << " [ online?" << isOnline()
             << "]) returning" << returnVal.size() << "bytes:" << returnVal;
  return returnVal;
}

void xtHelp::sError(QNetworkReply *rep)
{
  QVariant statCode = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  QVariant url = rep->url();
  if (DEBUG) qDebug() << "xtHelp Network Response Information-----------------------------";
  if (DEBUG) qDebug() << "xtHelp: http request url [" << url.toString() << "]";
  if (DEBUG) qDebug() << "xtHelp: status code received [" << statCode.toString() << "]";
  if(rep->error() == QNetworkReply::NoError)
  {
    if (DEBUG) qDebug() << "xtHelp: no error";
  }
  else
  {
    if (DEBUG) qDebug() << "xtHelp: error received";
    if (DEBUG) qDebug() << "xtHelp: " << rep->errorString();
  }
  if (DEBUG) qDebug() << "----------------------------------------------------------------";
}
