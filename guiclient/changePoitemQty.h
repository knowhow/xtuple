/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHANGEPOITEMQTY_H
#define CHANGEPOITEMQTY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_changePoitemQty.h"

class changePoitemQty : public XDialog, public Ui::changePoitemQty
{
    Q_OBJECT

public:
    changePoitemQty(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~changePoitemQty();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sPopulatePoitem( int pPoheadid );
    virtual void sPopulate( int pPoitemid );
    virtual void sChangeQty();
    virtual void sQtyChanged();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    double _cacheFreight;

};

#endif // CHANGEPOITEMQTY_H
