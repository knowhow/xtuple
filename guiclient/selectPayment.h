/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SELECTPAYMENT_H
#define SELECTPAYMENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_selectPayment.h"

class selectPayment : public XDialog, public Ui::selectPayment
{
    Q_OBJECT

public:
    selectPayment(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~selectPayment();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();
    virtual void sDiscount();
    virtual void sPriceGroup();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _apopenid;
    int _apselectid;

};

#endif // SELECTPAYMENT_H
