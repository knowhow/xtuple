/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBVIEWPROTO_H__
#define __QWEBVIEWPROTO_H__

#include <QObject>
#include <QtScript>
#include <QIcon>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QPainter>
#include <QWebPage>
#include <QUrl>
#include <QString>
#include <QWebView>

class QAction;
class QWebHistory;
class QWebSettings;

Q_DECLARE_METATYPE(QWebView*)

void setupQWebViewProto(QScriptEngine *engine);
QScriptValue constructQWebView(QScriptContext *context, QScriptEngine *engine);

class QWebViewProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebViewProto(QObject *parent);

    Q_INVOKABLE bool            findText(const QString &subString, QWebPage::FindFlags options = 0);
    Q_INVOKABLE QWebHistory*    history()       const;
    Q_INVOKABLE void            load(const QUrl &url);
    Q_INVOKABLE void            load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation, const QByteArray &body = QByteArray());
    Q_INVOKABLE QWebPage       *page()                                  const;
    Q_INVOKABLE QAction        *pageAction(QWebPage::WebAction action)  const;
    Q_INVOKABLE void            setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());
    Q_INVOKABLE void            setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    Q_INVOKABLE void            setPage(QWebPage *page);
    Q_INVOKABLE void            setTextSizeMultiplier(qreal factor);
    Q_INVOKABLE QWebSettings   *settings()              const;
    Q_INVOKABLE qreal           textSizeMultiplier()    const;
    Q_INVOKABLE void            triggerPageAction(QWebPage::WebAction action, bool checked = false);
};

#endif
