/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ACCOUNTINGYEARPERIODS_H
#define ACCOUNTINGYEARPERIODS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_accountingYearPeriods.h"

class accountingYearPeriods : public XWidget, public Ui::accountingYearPeriods
{
    Q_OBJECT

public:
    accountingYearPeriods(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~accountingYearPeriods();

public slots:
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sClosePeriod();
    virtual void sOpenPeriod();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // ACCOUNTINGYEARPERIODS_H
