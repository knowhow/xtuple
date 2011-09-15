/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSTACKEDWIDGETPROTO_H__
#define __QSTACKEDWIDGETPROTO_H__

#include <QObject>
#include <QStackedWidget>
#include <QtScript>

Q_DECLARE_METATYPE(QStackedWidget*)
//Q_DECLARE_METATYPE(QStackedWidget)

void setupQStackedWidgetProto(QScriptEngine *engine);
QScriptValue constructQStackedWidget(QScriptContext *context, QScriptEngine *engine);

class QStackedWidgetProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QStackedWidgetProto(QObject *parent);

    Q_INVOKABLE int      addWidget(QWidget *widget);
    Q_INVOKABLE int      count()                  const;
    Q_INVOKABLE int      currentIndex()           const;
    Q_INVOKABLE QWidget *currentWidget()          const;
    Q_INVOKABLE int      indexOf(QWidget *widget) const;
    Q_INVOKABLE int      insertWidget(int index, QWidget *widget);
    Q_INVOKABLE void     removeWidget(QWidget *widget);
    Q_INVOKABLE QWidget *widget(int index)        const;

    //Q_INVOKABLE QString toString()                const;
};

#endif
