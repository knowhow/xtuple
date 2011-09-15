/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINVOICEINFORMATION_H
#define DSPINVOICEINFORMATION_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspInvoiceInformation.h"

class dspInvoiceInformation : public XWidget, public Ui::dspInvoiceInformation
{
    Q_OBJECT

public:
    dspInvoiceInformation(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspInvoiceInformation();
    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set(const ParameterList & pParams );

protected slots:
    virtual void languageChange();

    virtual void sParseInvoiceNumber();
    virtual void sPrint();
    virtual void sViewDetails();
    virtual void sInvoiceList();

private:
    int _invcheadid;

};

#endif // DSPINVOICEINFORMATION_H
