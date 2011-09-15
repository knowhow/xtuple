/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits) * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef SELECTORDERFORBILLING_H
#define SELECTORDERFORBILLING_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_selectOrderForBilling.h"

class selectOrderForBilling : public XWidget, public Ui::selectOrderForBilling
{
    Q_OBJECT

public:
    selectOrderForBilling(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~selectOrderForBilling();

public slots:
    virtual SetResponse set( const ParameterList & pParams );

    virtual void clear();
    virtual void sCalculateTax();
    virtual void sCancelSelection();
    virtual void sEditOrder();
    virtual void sEditSelection();
    virtual void sFillList();
    virtual void sHandleShipchrg( int pShipchrgid );
    virtual void sPopulate( int pSoheadid );
    virtual void sSave();
    virtual void sSelectBalance();
    virtual void sSoList();
    virtual void sUpdateTotal();
    virtual void sTaxZoneChanged();
    virtual void sFreightChanged();

protected slots:
    virtual void languageChange();

    virtual void sTaxDetail();

protected:
    virtual void closeEvent(QCloseEvent * event);

private:
    int _cobmiscid;
    bool _captive;
    bool _updated;
    int _mode;
    int	_taxzoneidCache;
    double _freightCache;
};

#endif // SELECTORDERFORBILLING_H
