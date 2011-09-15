/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTOOLBARPROTO_H__
#define __QTOOLBARPROTO_H__

#include <QToolBar>
#include <QtScript>

Q_DECLARE_METATYPE(QToolBar*)
//Q_DECLARE_METATYPE(QToolBar)

void setupQToolBarProto(QScriptEngine *engine);
QScriptValue constructQToolBar(QScriptContext *context, QScriptEngine *engine);

class QToolBarProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QToolBarProto(QObject *parent);

    Q_INVOKABLE void addAction ( QAction * action );
    Q_INVOKABLE void addWidget ( QWidget * widget );
    Q_INVOKABLE QWidget * widgetForAction ( QAction * action ) const;
    Q_INVOKABLE QAction * insertWidget ( QAction * before, QWidget * widget );
    // TODO: fill in the rest of the function this class has
};

#endif
