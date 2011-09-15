/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebviewproto.h"

#include <QAction>
#include <QWebHistory>
#include <QWebSettings>
#include <QWebView>

QScriptValue QWebViewtoScriptValue(QScriptEngine *engine, QWebView* const &item)
{
  return engine->newQObject(item);
}

void QWebViewfromScriptValue(const QScriptValue &obj, QWebView* &item)
{
  item = qobject_cast<QWebView*>(obj.toQObject());
}

void setupQWebViewProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QWebViewtoScriptValue, QWebViewfromScriptValue);

  QScriptValue proto = engine->newQObject(new QWebViewProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebView*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebView,
                                                 proto);
  engine->globalObject().setProperty("QWebView",  constructor);
}

QScriptValue constructQWebView(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QWebView *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QWebView(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QWebView();
  return engine->toScriptValue(obj);
}

QWebViewProto::QWebViewProto(QObject *parent)
    : QObject(parent)
{
}

bool QWebViewProto::findText(const QString &subString, QWebPage::FindFlags options)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->findText(subString, options);
  return false;
}

QWebHistory* QWebViewProto::history() const
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->history();
  return 0;
}

void QWebViewProto::load(const QUrl &url)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->load(url);
}

void QWebViewProto::load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &body)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->load(request, operation, body);
}

QWebPage* QWebViewProto::page() const
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->page();
  return 0;
}

QAction* QWebViewProto::pageAction(QWebPage::WebAction action)  const
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->pageAction(action);
  return 0;
}

void QWebViewProto::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setContent(data, mimeType, baseUrl);
}

void QWebViewProto::setHtml(const QString &html, const QUrl &baseUrl)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setHtml(html, baseUrl);
}

void QWebViewProto::setPage(QWebPage *page)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setPage(page);
}

void QWebViewProto::setTextSizeMultiplier(qreal factor)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->setTextSizeMultiplier(factor);
}

QWebSettings* QWebViewProto::settings() const
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->settings();
  return 0;
}

qreal QWebViewProto::textSizeMultiplier() const
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    return item->textSizeMultiplier();
  return qreal();
}

void QWebViewProto::triggerPageAction(QWebPage::WebAction action, bool checked)
{
  QWebView *item = qscriptvalue_cast<QWebView*>(thisObject());
  if (item)
    item->triggerPageAction(action, checked);
}


