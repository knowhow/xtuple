/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPARTIALLYSHIPPEDORDERS_H
#define DSPPARTIALLYSHIPPEDORDERS_H

#include "display.h"

#include "ui_dspPartiallyShippedOrders.h"

class dspPartiallyShippedOrders : public display, public Ui::dspPartiallyShippedOrders
{
    Q_OBJECT

public:
    dspPartiallyShippedOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList&);

public slots:
    virtual void sHandlePrices( bool pShowPrices );
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sPrintPackingList();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int);

protected slots:
    virtual void languageChange();

};

#endif // DSPPARTIALLYSHIPPEDORDERS_H
