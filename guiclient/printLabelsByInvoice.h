/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTLABELSBYINVOICE_H
#define PRINTLABELSBYINVOICE_H

#include "xdialog.h"
#include "ui_printLabelsByInvoice.h"

class printLabelsByInvoice : public XDialog, public Ui::printLabelsByInvoice
{
    Q_OBJECT

public:
    printLabelsByInvoice(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printLabelsByInvoice();

public slots:
    virtual void sPrint();
    virtual void sParseInvoiceNumber();
    virtual void sSetToMin( int pValue );

protected slots:
    virtual void languageChange();

private:
    int _invcheadid;

};

#endif // PRINTLABELSBYINVOICE_H
