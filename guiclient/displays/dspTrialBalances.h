/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTRIALBALANCES_H
#define DSPTRIALBALANCES_H

#include "display.h"

class dspTrialBalances : public display
{
    Q_OBJECT

public:
    dspTrialBalances(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int);
    virtual void sViewTransactions();
    virtual void sForwardUpdate();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();
 
protected:
    virtual bool forwardUpdate();

};

#endif // DSPTRIALBALANCES_H
