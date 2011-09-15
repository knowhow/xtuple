/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ERRORLOG_H
#define ERRORLOG_H

#include "xwidget.h"
#include <xsqlquery.h>

#include "ui_errorLog.h"

class errorLog : public XWidget, public Ui::errorLog
{
    Q_OBJECT

public:
    errorLog(QWidget* parent = 0, const char * = 0, Qt::WFlags flags = 0);
    ~errorLog();

public slots:
    virtual void updateErrors(const QString &);

protected slots:
    virtual void languageChange();
    virtual void toggleDebug(bool);
    virtual void toggleWarning(bool);
    virtual void toggleCritical(bool);
    virtual void toggleFatal(bool);
};

class errorLogListener : public QObject, public XSqlQueryErrorListener {
  Q_OBJECT
  friend void xTupleMessageOutput(QtMsgType, const char*);

  public:
    errorLogListener(QObject * parent = 0);
    virtual ~errorLogListener();

    virtual void error(const QString &, const QSqlError&);
    static void initialize();
    static void destroy();

  public slots:
    void clear();

  signals:
    void updated(const QString &);
};


#endif // ERRORLOG_H
