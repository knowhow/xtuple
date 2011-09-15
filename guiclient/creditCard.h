/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREDITCARD_H
#define CREDITCARD_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_creditCard.h"

class creditCard : public XDialog, public Ui::creditCard
{
    Q_OBJECT

public:
    creditCard(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~creditCard();

    virtual SetResponse set(const ParameterList & pParams );
    static int saveCreditCard(QWidget *parent,
                              int &custId,
                              QString &ccName, 
                              QString &ccAddress1, 
                              QString &ccAddress2,
                              QString &ccCity, 
                              QString &ccState, 
                              QString &ccZip, 
                              QString &ccCountry,
                              QString &ccNumber,
                              QString &ccType,
                              QString &ccExpireMonth,
                              QString &ccExpireYear,
                              int &ccId,
                              bool ccActive = true );

public slots:
    virtual void sSave();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _ccardid;
    int _custid;
    int _mode;
};

#endif // CREDITCARD_H
