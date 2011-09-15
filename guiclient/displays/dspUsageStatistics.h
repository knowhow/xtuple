/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DSPUSAGESTATISTICS_H__
#define __DSPUSAGESTATISTICS_H__

#include "guiclient.h"
#include "display.h"

class dspUsageStatistics : public display
{
    Q_OBJECT

public:
    dspUsageStatistics(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList & params);
    virtual void viewTransactions(QString);

public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sViewAll();
    virtual void sViewReceipt();
    virtual void sViewIssue();
    virtual void sViewSold();
    virtual void sViewScrap();
    virtual void sViewAdjustment();
    virtual void sViewTransfer();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int pColumn );
    virtual void sPreview();
    virtual void sPrint();

private:
    bool _printing;

};

#endif // __DSPUSAGESTATISTICS_H__
