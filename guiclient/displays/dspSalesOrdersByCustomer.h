/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESORDERSBYCUSTOMER_H
#define DSPSALESORDERSBYCUSTOMER_H

#include "display.h"

#include "ui_dspSalesOrdersByCustomer.h"

class dspSalesOrdersByCustomer : public display, public Ui::dspSalesOrdersByCustomer
{
    Q_OBJECT

public:
    dspSalesOrdersByCustomer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool checkSitePrivs(int orderid);
    virtual bool setParams(ParameterList&);

public slots:
    virtual void sPopulatePo();
    virtual void sPopulateMenu( QMenu * menuThis, QTreeWidgetItem*, int);
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sCreateRA();
    virtual void sDspShipmentStatus();
    virtual void sDspShipments();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESORDERSBYCUSTOMER_H
