/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBPAGEPROTO_H__
#define __QWEBPAGEPROTO_H__

#include <QObject>
#include <QtScript>
#include <QWebPage>

Q_DECLARE_METATYPE(QWebPage*)
Q_DECLARE_METATYPE(enum QWebPage::LinkDelegationPolicy)

void setupQWebPageProto(QScriptEngine *engine);
QScriptValue constructQWebPage(QScriptContext *context, QScriptEngine *engine);

class QWebPageProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebPageProto(QObject *parent);

  // TO DO: add public functions
};

#endif
