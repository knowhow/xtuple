/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESORDERSBYITEM_H
#define DSPSALESORDERSBYITEM_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspSalesOrdersByItem.h"

class dspSalesOrdersByItem : public display, public Ui::dspSalesOrdersByItem
{
    Q_OBJECT

public:
    dspSalesOrdersByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool checkSitePrivs(int orderid);
    virtual bool setParams(ParameterList&);

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int);
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sCreateRA();
    virtual void sDspShipmentStatus();
    virtual void sDspShipments();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESORDERSBYITEM_H
