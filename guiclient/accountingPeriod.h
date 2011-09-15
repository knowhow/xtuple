/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ACCOUNTINGPERIOD_H
#define ACCOUNTINGPERIOD_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_accountingPeriod.h"

class accountingPeriod : public XDialog, public Ui::accountingPeriod
{
    Q_OBJECT

public:
    accountingPeriod(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~accountingPeriod();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sHandleNumber();
    virtual void sSave();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    bool _cachedFrozen;
    bool _cachedClosed;
    QDate _cachedEndDate;
    QDate _cachedStartDate;
    QString _cachedName;
    int _periodid;
    int _mode;
    int _cachedYearPeriodId;
    int _cachedQuarter;

};

#endif // ACCOUNTINGPERIOD_H
