/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ISSUETOSHIPPING_H
#define ISSUETOSHIPPING_H

#include "guiclient.h"

#include "xwidget.h"

#include <parameter.h>

#include "ui_issueToShipping.h"

class issueToShipping : public XWidget, public Ui::issueToShipping
{
    Q_OBJECT

public:
    issueToShipping(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~issueToShipping();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sBcFind();
    virtual void sCatchItemid( int pItemid );
    virtual void sCatchItemsiteid( int pItemsiteid );
    virtual void sCatchSoheadid( int pSoheadid );
    virtual void sCatchSoitemid( int pSoitemid );
    virtual void sCatchToheadid( int pToheadid );
    virtual void sCatchToitemid( int porderitemid );
    virtual void sCatchWoid( int pWoid );
    virtual void sHandleButtons();
    virtual void sIssueAllBalance();
    virtual void sIssueLineBalance();
    virtual bool sIssueLineBalance(int id, int altId);
    virtual void sIssueStock();
    virtual void sReturnStock();
    virtual void sShip();

protected slots:
    virtual void languageChange();
    virtual void sFillList();

private:
    bool	sufficientInventory(int);
    bool	sufficientItemInventory(int);

};

#endif // ISSUETOSHIPPING_H
