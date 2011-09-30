/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REPORTS_H
#define REPORTS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_reports.h"

class reports : public XWidget, public Ui::reports
{
    Q_OBJECT

public:
    reports(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~reports();

public slots:
    virtual bool setParams(ParameterList &params);
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // REPORTS_H
