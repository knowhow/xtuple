/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ADJUSTMENTTRANS_H
#define ADJUSTMENTTRANS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_adjustmentTrans.h"

class adjustmentTrans : public XWidget, public Ui::adjustmentTrans
{
    Q_OBJECT

public:
    adjustmentTrans(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = Qt::Window);
    ~adjustmentTrans();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPost();
    virtual void sPopulateQOH( int pWarehousid );
    virtual void sPopulateQty();
    virtual void sCostUpdated();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    double _cachedQOH;
    bool _captive;
    QButtonGroup* _adjustmentTypeGroupInt;
    QString _costMethod;

};

#endif // ADJUSTMENTTRANS_H
