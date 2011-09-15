/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xnetworkaccessmanager.h"

#include <QByteArray>
#include <QIODevice>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>

#define DEBUG false

// support functions
void setupQNetworkAccessManagerProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQNetworkAccessManagerProto entered");

  QScriptValue netmgrproto = engine->newQObject(new XNetworkAccessManager(engine));
  engine->setDefaultPrototype(qMetaTypeId<XNetworkAccessManager*>(),
                              netmgrproto);

  QScriptValue netmgrConstructor = engine->newFunction(constructQNetworkAccessManager,
                                                       netmgrproto);
  engine->globalObject().setProperty("QNetworkAccessManager",
                                     netmgrConstructor);
}

QScriptValue constructQNetworkAccessManager(QScriptContext *context,
                                            QScriptEngine  *engine)
{
  if (DEBUG) qDebug("constructQNetworkAccessManager called");
  XNetworkAccessManager *netmgr = 0;

  if (context->argumentCount() == 1)
  {
    if (DEBUG) qDebug("netmgr(1 arg)");
    netmgr = new XNetworkAccessManager(context->argument(0).toQObject());
  }
  else
    netmgr = new XNetworkAccessManager();

  return engine->toScriptValue(netmgr);
}

// XNetworkAccessManager itself
XNetworkAccessManager::XNetworkAccessManager(QObject *parent)
  : QNetworkAccessManager(parent)
{
}

QNetworkCookieJar *XNetworkAccessManager::cookieJar() const
{
  return QNetworkAccessManager::cookieJar();
}

QNetworkReply *XNetworkAccessManager::get(const QNetworkRequest &request)
{
  return QNetworkAccessManager::get(request);
}

QNetworkReply *XNetworkAccessManager::head(const QNetworkRequest &request)
{
  return QNetworkAccessManager::head(request);
}

QNetworkReply *XNetworkAccessManager::post(const QNetworkRequest &request,
                                                QIODevice *data)
{
  return QNetworkAccessManager::post(request, data);
}

QNetworkReply *XNetworkAccessManager::post(const QNetworkRequest &request,
                                                  const QByteArray &data)
{
  return QNetworkAccessManager::post(request, data);
}    

QNetworkReply *XNetworkAccessManager::post(const QNetworkRequest &request,
                                                const QString &data)
{
  if (DEBUG)
    qDebug("XNetworkAccessManager::post(QNetworkRequest, QString) entered");
  return QNetworkAccessManager::post(request, data.toAscii());
}    

QNetworkProxy XNetworkAccessManager::proxy() const
{
  return QNetworkAccessManager::proxy();
}

QNetworkReply *XNetworkAccessManager::put(const QNetworkRequest &request,
                                               QIODevice *data)
{
  return QNetworkAccessManager::put(request, data);
}

QNetworkReply * XNetworkAccessManager::put(const QNetworkRequest &request,
                                                const QByteArray &data)
{
  return QNetworkAccessManager::put(request, data);
}

void XNetworkAccessManager::setCookieJar(QNetworkCookieJar *cookieJar)
{
  QNetworkAccessManager::setCookieJar(cookieJar);
}

void XNetworkAccessManager::setProxy(const QNetworkProxy &proxy)
{
  QNetworkAccessManager::setProxy(proxy);
}

QString XNetworkAccessManager::toString() const
{
  return QString("[QNetworkAccessManager(%1)]")
      .arg(parent() ? parent()->objectName() : QString("unparented"));
}
