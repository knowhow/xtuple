/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ARWORKBENCH_H
#define ARWORKBENCH_H

#include "guiclient.h"

#include "xwidget.h"
#include <QMenu>

#include <parameter.h>

#include "ui_arWorkBench.h"

#include "dspAROpenItems.h"
#include "dspCreditCardTransactions.h"

class arWorkBench : public XWidget, public Ui::arWorkBench
{
    Q_OBJECT

public:
    arWorkBench(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~arWorkBench();

    virtual SetResponse set( const ParameterList & pParams );

public slots:
    virtual bool setParams(ParameterList &params);
    virtual void sDeleteCashrcpt();
    virtual void sEditCashrcpt();
    virtual void sFillCashrcptList();
    virtual void sFillList();
    virtual void sNewCashrcpt();
    virtual void sPopulateCashRcptMenu(QMenu*);
    virtual void sPostCashrcpt();
    virtual void sViewCashrcpt();
    virtual void sClear();
    virtual void sSearchDocNumChanged();
    
protected:
    dspAROpenItems *_aritems;
    dspCreditCardTransactions *_cctrans;
    
protected slots:
    virtual void languageChange();

};

#endif // ARWORKBENCH_H
