/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDPURCHASEORDERS_H
#define UNPOSTEDPURCHASEORDERS_H

#include "display.h"

#include "ui_unpostedPurchaseOrders.h"

class unpostedPurchaseOrders : public display, public Ui::unpostedPurchaseOrders
{
    Q_OBJECT

public:
    unpostedPurchaseOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sNew();
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*, int);
    virtual void sRelease();
    virtual void sPrint();
    virtual void sView();
    virtual bool setParams(ParameterList &);
};

#endif // UNPOSTEDPURCHASEORDERS_H
