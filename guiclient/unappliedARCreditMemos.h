/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNAPPLIEDARCREDITMEMOS_H
#define UNAPPLIEDARCREDITMEMOS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_unappliedARCreditMemos.h"

class unappliedARCreditMemos : public XWidget, public Ui::unappliedARCreditMemos
{
    Q_OBJECT

public:
    unappliedARCreditMemos(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unappliedARCreditMemos();

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sView();
    virtual void sPopulateMenu( QMenu * );
    virtual void sFillList();
    virtual void sApply();

protected slots:
    virtual void languageChange();

};

#endif // UNAPPLIEDARCREDITMEMOS_H
