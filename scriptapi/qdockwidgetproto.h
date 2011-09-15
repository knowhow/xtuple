/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOCKWIDGETPROTO_H__
#define __QDOCKWIDGETPROTO_H__

#include <QObject>
#include <QtScript>
#include <QDockWidget>

Q_DECLARE_METATYPE(QDockWidget*)

void setupQDockWidgetProto(QScriptEngine *engine);
QScriptValue constructQDockWidget(QScriptContext *context, QScriptEngine *engine);

class QDockWidgetProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDockWidgetProto(QObject *parent);

    Q_INVOKABLE void	setTitleBarWidget ( QWidget * widget );
    Q_INVOKABLE void	setWidget ( QWidget * widget );
    Q_INVOKABLE QWidget *	titleBarWidget () const;
    Q_INVOKABLE QAction *	toggleViewAction () const;
    Q_INVOKABLE QWidget *	widget () const;
};

#endif
