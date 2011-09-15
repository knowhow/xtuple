/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SUBMITREPORT_H
#define SUBMITREPORT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_submitReport.h"

class submitReport : public XDialog, public Ui::submitReport
{
    Q_OBJECT

public:
    submitReport(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~submitReport();

    Q_INVOKABLE virtual int check();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    QString _cachedReportName;
    int _error;
    ParameterList _params;

};

#endif // SUBMITREPORT_H
