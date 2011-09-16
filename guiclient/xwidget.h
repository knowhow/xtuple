/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#ifndef __XWIDGET_H__
#define __XWIDGET_H__

#include <QWidget>

#include <parameter.h>
#include <guiclient.h>

class QScriptEngine;
class XWidgetPrivate;

class XWidget : public QWidget
{
  Q_OBJECT

  public:
    XWidget(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    XWidget(QWidget * parent, const char * name, Qt::WindowFlags flags = 0);
    ~XWidget();

    Q_INVOKABLE virtual ParameterList get() const;

  public slots:
    virtual enum SetResponse set(const ParameterList &);

  protected:
    void closeEvent(QCloseEvent * event);
    void showEvent(QShowEvent * event);
    QScriptEngine *engine();

  protected slots:
    virtual enum SetResponse postSet();

  private:
    friend class XWidgetPrivate;
    XWidgetPrivate *_private;

    ParameterList _lastSetParams;
    void loadScriptEngine();
};

#endif // __XWIDGET_H__

