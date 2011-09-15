/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef INVOICE_H
#define INVOICE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_invoice.h"

class invoice : public XWidget, public Ui::invoice
{
    Q_OBJECT

public:
    invoice(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~invoice();

    static void newInvoice( int pCustid, QWidget *parent = 0 );
    static void editInvoice( int pId, QWidget *parent = 0 );
    static void viewInvoice( int pId, QWidget *parent = 0 );

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sClose();
    virtual void sPopulateCustomerInfo( int pCustid );
    virtual void populateShipto( int pShiptoid );
    virtual void sCopyToShipto();
    virtual void sSave();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void populate();
    virtual void sFillItemList();
    virtual void sCalculateTotal();
    virtual void sCalculateTax();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void setFreeFormShipto( bool pFreeForm );
    virtual void sShipToModified();
    virtual void populateCMInfo();
    virtual void populateCCInfo();
    virtual bool sCheckInvoiceNumber();
    virtual void sHandleShipchrg( int pShipchrgid );
    virtual void sTaxZoneChanged();
    virtual void sFreightChanged();

protected:
    virtual void keyPressEvent( QKeyEvent * e );

protected slots:
    virtual void languageChange();
    virtual void sTaxDetail();

private:
    int		_mode;
    double	_cachedSubtotal;
    int		_custtaxzoneid;
    bool	_ffShipto;
    int		_invcheadid;
    int		_taxzoneidCache;
    bool        _loading;
    double      _freightCache;
    bool        save();
    bool        _posted;

};

#endif // INVOICE_H
