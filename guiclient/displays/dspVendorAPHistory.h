/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPVENDORAPHISTORY_H
#define DSPVENDORAPHISTORY_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspVendorAPHistory.h"

class dspVendorAPHistory : public display, public Ui::dspVendorAPHistory
{
    Q_OBJECT

public:
    dspVendorAPHistory(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected, int);
    virtual void sEdit();
    virtual void sView();
    virtual void sVoidVoucher();
    virtual void sViewVoucher();
    virtual void sViewGLSeries();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

    virtual void sSearchInvoiceNum();


};

#endif // DSPVENDORAPHISTORY_H
