/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESHISTORYBYBILLTONAME_H
#define DSPSALESHISTORYBYBILLTONAME_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspSalesHistoryByBilltoName.h"

class dspSalesHistoryByBilltoName : public display, public Ui::dspSalesHistoryByBilltoName
{
    Q_OBJECT

public:
    dspSalesHistoryByBilltoName(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList&);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sHandleParams();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int);
    virtual void sEdit();
    virtual void sView();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESHISTORYBYBILLTONAME_H
