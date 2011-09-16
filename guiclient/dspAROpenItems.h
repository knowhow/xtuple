/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPAROPENITEMS_H
#define DSPAROPENITEMS_H

#include "display.h"
#include <parameter.h>

#include "ui_dspAROpenItems.h"

class dspAROpenItems : public display, public Ui::dspAROpenItems
{
    Q_OBJECT

public:
    dspAROpenItems(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    
    Q_INVOKABLE bool checkInvoiceSitePrivs(int);
    Q_INVOKABLE bool checkCreditMemoSitePrivs(int);
    Q_INVOKABLE bool checkSalesOrderSitePrivs(int);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool setParams(ParameterList &);
    virtual void sApplyAropenCM();
    virtual void sCCRefundCM();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *pItem, int );
    virtual void sDeleteCreditMemo();
    virtual void sDeleteInvoice();
    virtual void sDspShipmentStatus();
    virtual void sEdit();
    virtual void sEditSalesOrder();
    virtual void sEnterMiscArCreditMemo();
    virtual void sEnterMiscArDebitMemo();
    virtual void sCreateInvoice();
    virtual void sNewCashrcpt();
    virtual void sNewCreditMemo();
    virtual void sView();
    virtual void sViewCreditMemo();
    virtual void sVoidCreditMemo();
    virtual void sViewInvoice();
    virtual void sVoidInvoiceDetails();
    virtual void sEditInvoiceDetails();
    virtual void sViewInvoiceDetails();
    virtual void sViewSalesOrder();
    virtual void sIncident();
    virtual void sViewIncident();
    virtual void sPrint();
    virtual void sPrintItem();
    virtual void sPost();
    virtual void sPostInvoice();
    virtual void sPostCreditMemo();
    virtual void sShipment();
    virtual void sHandleReportName();
    virtual void sHandlePrintGroup();
    virtual void sClosedToggled(bool);

signals:
    void finishedPrintingStatement(int);
};

#endif // DSPAROPENITEMS_H
