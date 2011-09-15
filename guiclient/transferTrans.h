/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSFERTRANS_H
#define TRANSFERTRANS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_transferTrans.h"

class transferTrans : public XWidget, public Ui::transferTrans
{
    Q_OBJECT

public:
    transferTrans(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~transferTrans();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );

protected slots:
    virtual void languageChange();

    virtual void sPost();
    virtual void sPopulateFromQty( int pWarehousid );
    virtual void sPopulateToQty( int pWarehousid );
    virtual void sUpdateQty( const QString & pQty );


private:
    bool _captive;
    int _mode;
    double _cachedFromBeforeQty;
    double _cachedToBeforeQty;

};

#endif // TRANSFERTRANS_H
