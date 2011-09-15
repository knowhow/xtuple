/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSHIPPINGFORM_H
#define PRINTSHIPPINGFORM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_printShippingForm.h"

class printShippingForm : public XDialog, public Ui::printShippingForm
{
    Q_OBJECT

public:
    printShippingForm(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = 0);
    ~printShippingForm();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sEditShippingFormWatermark();
    virtual void sHandleShipment();
    virtual void sHandleShippingFormCopies( int pValue );
    virtual void sHandleOrder();
    virtual void sHandleTo();
    virtual void sHandleSo();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    virtual void depopulate();

};

#endif // PRINTSHIPPINGFORM_H
