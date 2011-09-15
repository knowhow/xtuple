/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDINVOICES_H
#define UNPOSTEDINVOICES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_unpostedInvoices.h"

class unpostedInvoices : public XWidget, public Ui::unpostedInvoices
{
    Q_OBJECT

public:
    unpostedInvoices(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unpostedInvoices();
    virtual bool checkSitePrivs(int invcid);

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPrint();
    virtual void sPost();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // UNPOSTEDINVOICES_H
