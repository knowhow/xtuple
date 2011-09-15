/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qnetworkrequestproto.h"

#include <QByteArray>
#include <QList>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>

#define DEBUG false

void setupQNetworkRequestProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQNetworkRequestProto entered");

  QScriptValue netreqproto = engine->newQObject(new QNetworkRequestProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), netreqproto);
  QScriptValue netreqConstructor = engine->newFunction(constructQNetworkRequest,
                                                       netreqproto);
  engine->globalObject().setProperty("QNetworkRequest", netreqConstructor);
}

QScriptValue constructQNetworkRequest(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  if (DEBUG) qDebug("constructQNetworkRequest called");
  QNetworkRequest *req = 0;

  if (context->argumentCount() > 0)
    context->throwError(QScriptContext::UnknownError,
                        "QNetworkRequest() constructors with "
                        "arguments are not supported");
  else
    req = new QNetworkRequest();

  return engine->toScriptValue(req);
}

QNetworkRequestProto::QNetworkRequestProto(QObject *parent)
  : QObject(parent)
{
}

QVariant QNetworkRequestProto::attribute(QNetworkRequest::Attribute code, const QVariant &defaultValue) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->attribute(code, defaultValue);
  return QVariant();
}

bool QNetworkRequestProto::hasRawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->hasRawHeader(headerName);
  return false;
}    

QVariant QNetworkRequestProto::header(QNetworkRequest::KnownHeaders header) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->header(header);
  return QVariant();
}

QByteArray QNetworkRequestProto::rawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->rawHeader(headerName);
  return QByteArray();
}

QList<QByteArray> QNetworkRequestProto::rawHeaderList() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->rawHeaderList();
  return QList<QByteArray>();
}

void QNetworkRequestProto::setAttribute(QNetworkRequest::Attribute code, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setAttribute(code, value);
}

void QNetworkRequestProto::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setHeader(header, value);
}

void QNetworkRequestProto::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setRawHeader(headerName, headerValue);
}

#ifndef QT_NO_OPENSSL
void QNetworkRequestProto::setSslConfiguration(const QSslConfiguration &config)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setSslConfiguration(config);
}

QSslConfiguration QNetworkRequestProto::sslConfiguration() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}
#endif

void QNetworkRequestProto::setUrl(const QUrl &url)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setUrl(url);
}

QUrl QNetworkRequestProto::url() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

QString QNetworkRequestProto::toString() const
{
  return QString("[QNetworkRequest(url=%1)]")
            .arg(url().toString(QUrl::RemovePassword));
}
