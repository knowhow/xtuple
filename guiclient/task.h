/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TASK_H
#define TASK_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_task.h"

class task : public XDialog, public Ui::task
{
    Q_OBJECT

public:
    task(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~task();

    virtual SetResponse set(const ParameterList & pParams );

public slots:
    virtual void populate();
    virtual void sSave();
    virtual void sNewUser();
    virtual void sDeleteUser();
    virtual void sFillUserList();
    virtual void sHoursAdjusted();
    virtual void sExpensesAdjusted();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _prjid;
    int _prjtaskid;
};

#endif // TASK_H
