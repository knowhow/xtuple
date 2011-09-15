/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef APPLYDISCOUNT_H
#define APPLYDISCOUNT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_applyDiscount.h"

class applyDiscount : public XDialog, public Ui::applyDiscount
{
    Q_OBJECT

public:
    applyDiscount(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~applyDiscount();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sApply();
    virtual void sViewVoucher();

protected:
    int _apopenid;

protected slots:
    virtual void languageChange();

    virtual void populate();

};

#endif // APPLYDISCOUNT_H
