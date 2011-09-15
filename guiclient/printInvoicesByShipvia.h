/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTINVOICESBYSHIPVIA_H
#define PRINTINVOICESBYSHIPVIA_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_printInvoicesByShipvia.h"

class printInvoicesByShipvia : public XDialog, public Ui::printInvoicesByShipvia
{
    Q_OBJECT

public:
    printInvoicesByShipvia(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printInvoicesByShipvia();

public slots:
    virtual void sPrint();
    virtual void sEditWatermark();
    virtual void sHandleCopies( int pValue );

signals:
            void finishedPrinting(int);

protected slots:
    virtual void languageChange();

};

#endif // PRINTINVOICESBYSHIPVIA_H
