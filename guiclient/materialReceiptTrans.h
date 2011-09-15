/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MATERIALRECEIPTTRANS_H
#define MATERIALRECEIPTTRANS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_materialReceiptTrans.h"

class materialReceiptTrans : public XWidget, public Ui::materialReceiptTrans
{
    Q_OBJECT

public:
    materialReceiptTrans(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~materialReceiptTrans();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPost();
    virtual void sPopulateQty();
    virtual void sUpdateQty( const QString & pQty );
    virtual void sCostUpdated();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    double _cachedQOH;
    bool _captive;

};

#endif // MATERIALRECEIPTTRANS_H
