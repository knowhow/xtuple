/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREDITMEMO_H
#define CREDITMEMO_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_creditMemo.h"

class creditMemo : public XWidget, public Ui::creditMemo
{
    Q_OBJECT

public:
    creditMemo(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~creditMemo();

    virtual void setNumber();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCalculateTax();
    virtual void sSave();
    virtual void sInvoiceList();
    virtual void populateShipto( int pShiptoid );
    virtual void sPopulateCustomerInfo();
    virtual void sCheckCreditMemoNumber();
    virtual void sConvertShipto();
    virtual void sCopyToShipto();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sCalculateSubtotal();
    virtual void sCalculateTotal();
    virtual void populate();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sTaxZoneChanged();
    virtual void sTaxDetail();
    virtual void sFreightChanged();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _cmheadid;
    int	_custtaxzoneid;
    bool _ffBillto;
    bool _ffShipto;
    bool _ignoreShiptoSignals;
    double _subtotalCache;
    double _freightCache;
    int _taxzoneidCache;
    bool save();
};

#endif // CREDITMEMO_H
