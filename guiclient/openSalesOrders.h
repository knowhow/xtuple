/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPENSALESORDERS_H
#define OPENSALESORDERS_H

#include "display.h"
#include <parameter.h>

#include "ui_openSalesOrders.h"

class openSalesOrders : public display, public Ui::openSalesOrders
{
    Q_OBJECT

public:
    openSalesOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    
    Q_INVOKABLE virtual void setCustId(int custId);
    Q_INVOKABLE virtual int custId();
    Q_INVOKABLE virtual bool checkSitePrivs();

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sPrintPackingList();
    virtual void sAddToPackingListBatch();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sPrintForms(); 
    virtual void sIssueToShipping();
    virtual void sDspShipmentStatus();
    virtual void sShipment();
    virtual bool setParams(ParameterList &);

private:
    int _custid;
};

#endif // OPENSALESORDERS_H
