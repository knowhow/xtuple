/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPENPURCHASEORDER_H
#define OPENPURCHASEORDER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_openPurchaseOrder.h"
class openPurchaseOrder : public XDialog, public Ui::openPurchaseOrder
{
    Q_OBJECT

public:
    openPurchaseOrder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~openPurchaseOrder();
    //virtual bool setParams(ParameterList &);
    int vendor_id;

public slots:

    virtual void sFillList();
    virtual void sSelect();
    virtual enum SetResponse set( ParameterList & pParams );
protected slots:
    virtual void languageChange();

};

#endif // OPENPURCHASEORDER_H
