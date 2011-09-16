/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTINVOICE_H
#define PRINTINVOICE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_printInvoice.h"

class printInvoice : public XDialog, public Ui::printInvoice
{
    Q_OBJECT

public:
    printInvoice(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printInvoice();

    Q_INVOKABLE virtual int  id();
    Q_INVOKABLE virtual bool isSetup();
    Q_INVOKABLE virtual void populate();
    Q_INVOKABLE virtual void setSetup(bool);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sPrint();
    virtual void sHandleCopies( int pValue );
    virtual void sEditWatermark();

signals:
            void finishedPrinting(int);

protected slots:
    virtual void languageChange();

private:
    bool      _alert;
    bool      _captive;
    int       _invcheadid;
    QPrinter *_printer;
    bool      _setup;

};

#endif // PRINTINVOICE_H
