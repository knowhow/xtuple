/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCREDITCARDTRANSACTIONS_H
#define DSPCREDITCARDTRANSACTIONS_H

#include "guiclient.h"

#include "xwidget.h"
#include <QMenu>

#include <parameter.h>

#include "ui_dspCreditCardTransactions.h"

class dspCreditCardTransactions : public XWidget, public Ui::dspCreditCardTransactions
{
    Q_OBJECT

public:
    dspCreditCardTransactions(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCreditCardTransactions();

    virtual SetResponse set( const ParameterList & pParams );

public slots:
    virtual void sFillList();
    virtual void sPostPreauth();
    virtual void sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* pItem);
    virtual void sPrintCCReceipt();
    virtual void sVoidPreauth();
    virtual void sgetCCAmount();
    virtual void sClear();
    
protected slots:
    virtual void languageChange();

};

#endif // DSPCREDITCARDTRANSACTIONS_H
