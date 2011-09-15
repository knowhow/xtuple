/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CASHRECEIPTITEM_H
#define CASHRECEIPTITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_cashReceiptItem.h"

class cashReceiptItem : public XDialog, public Ui::cashReceiptItem
{
    Q_OBJECT

public:
    cashReceiptItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~cashReceiptItem();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();
    virtual void sDiscount();

protected slots:
    virtual void languageChange();

private:
    int _cashrcptid;
    int _cashrcptitemid;
    int _aropenid;
    int _mode;
	double _amttoapply;
};

#endif // CASHRECEIPTITEM_H
