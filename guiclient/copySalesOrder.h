/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COPYSALESORDER_H
#define COPYSALESORDER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_copySalesOrder.h"

class copySalesOrder : public XDialog, public Ui::copySalesOrder
{
    Q_OBJECT

public:
    copySalesOrder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~copySalesOrder();

public slots:
    virtual SetResponse set( ParameterList & pParams );
    virtual void sPopulateSoInfo( int pSoid );
    virtual void sCopy();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // COPYSALESORDER_H
