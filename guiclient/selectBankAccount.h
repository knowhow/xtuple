/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SELECTBANKACCOUNT_H
#define SELECTBANKACCOUNT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_selectBankAccount.h"

class selectBankAccount : public XDialog, public Ui::selectBankAccount
{
    Q_OBJECT

public:
    selectBankAccount(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~selectBankAccount();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sSelect();
    virtual void sClose();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _bankaccntid;
    QString _type;

};

#endif // SELECTBANKACCOUNT_H
