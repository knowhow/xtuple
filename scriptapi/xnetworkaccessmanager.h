/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XNETWORKACCESSMANAGER_H__
#define __XNETWORKACCESSMANAGER_H__

#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>
#include <QtScript>

class QByteArray;
class QIODevice;
class QNetworkCookieJar;
class QNetworkReply;
class QNetworkRequest;
class QSslError;

void setupQNetworkAccessManagerProto(QScriptEngine *engine);
QScriptValue constructQNetworkAccessManager(QScriptContext *context,
                                            QScriptEngine *engine);

class XNetworkAccessManager : public QNetworkAccessManager
{
  Q_OBJECT

  public:
    XNetworkAccessManager(QObject *parent = 0);

    Q_INVOKABLE QNetworkCookieJar *cookieJar() const;
    Q_INVOKABLE QNetworkReply     *get(const QNetworkRequest &request);
    Q_INVOKABLE QNetworkReply     *head(const QNetworkRequest &request);
    Q_INVOKABLE QNetworkReply     *post(const QNetworkRequest &request,
                                        QIODevice *data);
    Q_INVOKABLE QNetworkReply     *post(const QNetworkRequest &request,
                                        const QByteArray &data);
    Q_INVOKABLE QNetworkReply     *post(const QNetworkRequest &request,
                                        const QString &data);
    Q_INVOKABLE QNetworkProxy      proxy() const;
    Q_INVOKABLE QNetworkReply     *put(const QNetworkRequest &request,
                                       QIODevice *data);
    Q_INVOKABLE QNetworkReply     *put(const QNetworkRequest &request,
                                       const QByteArray &data);
    Q_INVOKABLE void               setCookieJar(QNetworkCookieJar *cookieJar);
    Q_INVOKABLE void               setProxy(const QNetworkProxy &proxy);
    Q_INVOKABLE QString            toString() const;
};

Q_DECLARE_METATYPE(XNetworkAccessManager*)

#endif
