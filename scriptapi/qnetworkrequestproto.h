/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QNETWORKREQUESTPROTO_H__
#define __QNETWORKREQUESTPROTO_H__

#include <QNetworkRequest>
#include <QObject>
#include <QSslConfiguration>
#include <QtScript>

class QByteArray;

Q_DECLARE_METATYPE(QNetworkRequest*)

void setupQNetworkRequestProto(QScriptEngine *engine);
QScriptValue constructQNetworkRequest(QScriptContext *context,
                                      QScriptEngine *engine);

class QNetworkRequestProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QNetworkRequestProto(QObject *parent);

    Q_INVOKABLE QVariant  attribute(const QNetworkRequest::Attribute code,
                                    const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE bool      hasRawHeader(const QByteArray &headerName)      const;
    Q_INVOKABLE QVariant  header(QNetworkRequest::KnownHeaders header)    const;
    Q_INVOKABLE QByteArray        rawHeader(const QByteArray &headerName) const;
    Q_INVOKABLE QList<QByteArray> rawHeaderList()                         const;
    Q_INVOKABLE void              setAttribute(QNetworkRequest::Attribute code,
                                               const QVariant &value);
    Q_INVOKABLE void              setHeader(QNetworkRequest::KnownHeaders header,
                                            const QVariant &value);
    Q_INVOKABLE void              setRawHeader(const QByteArray &headerName,
                                               const QByteArray &headerValue);
#ifndef QT_NO_OPENSSL
    Q_INVOKABLE void              setSslConfiguration(const QSslConfiguration &config);
    Q_INVOKABLE QSslConfiguration sslConfiguration() const;
#endif
    Q_INVOKABLE void              setUrl(const QUrl &url);
    Q_INVOKABLE QString           toString()         const;
    Q_INVOKABLE QUrl              url()              const;
};

#endif
