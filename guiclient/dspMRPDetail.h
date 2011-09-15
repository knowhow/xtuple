/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPMRPDETAIL_H
#define DSPMRPDETAIL_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspMRPDetail.h"

class dspMRPDetail : public XWidget, public Ui::dspMRPDetail
{
    Q_OBJECT

public:
    dspMRPDetail(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspMRPDetail();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *, int pColumn );
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sIssuePR();
    virtual void sIssuePO();
    virtual void sIssueWO();
    virtual void sFillItemsites();
    virtual void sFillMRPDetail();

protected slots:
    virtual void languageChange();

private:
    int _column;

};

#endif // DSPMRPDETAIL_H
