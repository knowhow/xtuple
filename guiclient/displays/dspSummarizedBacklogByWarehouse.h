/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUMMARIZEDBACKLOGBYWAREHOUSE_H
#define DSPSUMMARIZEDBACKLOGBYWAREHOUSE_H

#include "display.h"

#include "ui_dspSummarizedBacklogByWarehouse.h"

class dspSummarizedBacklogByWarehouse : public display, public Ui::dspSummarizedBacklogByWarehouse
{
    Q_OBJECT

public:
    dspSummarizedBacklogByWarehouse(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sHandlePrices( bool pShowPrices );
    virtual void sInventoryAvailabilityBySalesOrder();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPrintPackingList();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int);
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPSUMMARIZEDBACKLOGBYWAREHOUSE_H
