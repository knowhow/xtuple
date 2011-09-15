/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDCREDITMEMOS_H
#define UNPOSTEDCREDITMEMOS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_unpostedCreditMemos.h"

class unpostedCreditMemos : public XWidget, public Ui::unpostedCreditMemos
{
    Q_OBJECT

public:
    unpostedCreditMemos(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unpostedCreditMemos();
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual void sPopulateMenu(QMenu * menuThis);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sPrint();
    virtual void sPost();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // UNPOSTEDCREDITMEMOS_H
