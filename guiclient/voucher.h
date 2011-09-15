/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VOUCHER_H
#define VOUCHER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_voucher.h"

class voucher : public XWidget, public Ui::voucher
{
    Q_OBJECT

public:
    voucher(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~voucher();

    Q_INVOKABLE void enableWindowModifiedSetting();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool sSave();
    virtual void sHandleVoucherNumber();
    virtual void sPopulate();
    virtual void sDataChanged();
    virtual void sDistributions();
    virtual void sDistributeLine();
    virtual void sClear();
    virtual void sDistributeAll();
    virtual void sNewMiscDistribution();
    virtual void sEditMiscDistribution();
    virtual void sDeleteMiscDistribution();
    virtual void sFillList();
    virtual void sFillMiscList();
    virtual void sPopulatePoInfo();
    virtual void sPopulateDistributed();
    virtual void sPopulateBalanceDue();
    virtual void populateNumber();
    virtual void populate();
    virtual void clear();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sPopulateDistDate();
    virtual void sPopulateDueDate();
    virtual void sView();
    virtual void saveDetail();

protected:
    virtual void keyPressEvent( QKeyEvent * e );

protected slots:
    virtual void languageChange();

    virtual void sPopulateMenu( QMenu * pMenu );


private:
    int _vendid;
    int _voheadid;
    int _mode;

};

#endif // VOUCHER_H
