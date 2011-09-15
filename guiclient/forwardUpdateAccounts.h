/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FORWARDUPDATEACCOUNTS_H
#define FORWARDUPDATEACCOUNTS_H

#include "xdialog.h"

#include "ui_forwardUpdateAccounts.h"

class forwardUpdateAccounts : public XDialog, public Ui::forwardUpdateAccounts
{
    Q_OBJECT

public:
    forwardUpdateAccounts(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~forwardUpdateAccounts();

public slots:
    virtual void sUpdate();

protected slots:
    virtual void languageChange();

};

#endif // FORWARDUPDATEACCOUNTS_H
