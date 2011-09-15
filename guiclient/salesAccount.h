/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALESACCOUNT_H
#define SALESACCOUNT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_salesAccount.h"

class salesAccount : public XDialog, public Ui::salesAccount
{
    Q_OBJECT

public:
    salesAccount(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~salesAccount();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sSave();


private:
    int _mode;
    int _salesaccntid;

};

#endif // SALESACCOUNT_H
