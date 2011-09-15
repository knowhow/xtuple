/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIPTO_H
#define SHIPTO_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "contactcluster.h"
#include "ui_shipTo.h"

class shipTo : public XDialog, public Ui::shipTo
{
    Q_OBJECT

public:
    shipTo(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~shipTo();

    Q_INVOKABLE virtual int id() const;

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();
    virtual void sPopulateNumber();
    virtual void sPopulateCommission( int pSalesrepid );
    virtual void closeEvent( QCloseEvent * pEvent );

signals:
            void newId(int);
            void populated();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _custid;
    int _shiptoid;
    bool _inTransaction;

};

#endif // SHIPTO_H
