/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RETURNAUTHCHECK_H
#define RETURNAUTHCHECK_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_returnAuthCheck.h"

class returnAuthCheck : public XDialog, public Ui::returnAuthCheck
{
    Q_OBJECT

public:
    returnAuthCheck(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~returnAuthCheck();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sClose();
	virtual void sPopulateBankInfo(int pBankaccntid);
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _bankaccntid;
	int _checkid;
	int _custid;
	int _aropenid;
    int _cmheadid;
    int _cmheadcurrid;

};

#endif // RETURNAUTHCHECK_H
