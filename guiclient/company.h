/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COMPANY_H
#define COMPANY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_company.h"

class company : public XDialog, public Ui::company
{
    Q_OBJECT

public:
    company(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~company();

public slots:
    virtual SetResponse set(const  ParameterList & pParams );
    virtual void sSave();
    virtual void populate();

protected slots:
    virtual void languageChange();
    virtual void sHandleTest();
    virtual void sTest();
    virtual void sCurrencyChanged();

private:
    int _mode;
    int _companyid;
    QString _cachedNumber;
    int _cachedCurrid;

};

#endif // COMPANY_H
