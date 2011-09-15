/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MISCVOUCHER_H
#define MISCVOUCHER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_miscVoucher.h"

class miscVoucher : public XWidget, public Ui::miscVoucher
{
    Q_OBJECT

public:
    miscVoucher(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~miscVoucher();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sHandleVoucherNumber();
    virtual void sPopulateVendorInfo( int pVendid );
    virtual void sNewMiscDistribution();
    virtual void sEditMiscDistribution();
    virtual void sDeleteMiscDistribution();
    virtual void sFillMiscList();
    virtual void sPopulateDistributed();
    virtual void sPopulateBalanceDue();
    virtual void populateNumber();
    virtual void populate();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sPopulateDistDate();
    virtual void sPopulateDueDate();

protected:
    virtual void keyPressEvent( QKeyEvent * e );

protected slots:
    virtual void languageChange();

private:
    int _voheadid;
    double _cachedAmountDistributed;
    int _mode;
};

#endif // MISCVOUCHER_H
