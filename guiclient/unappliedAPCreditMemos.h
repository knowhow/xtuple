/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNAPPLIEDAPCREDITMEMOS_H
#define UNAPPLIEDAPCREDITMEMOS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_unappliedAPCreditMemos.h"

class ParameterList;

class unappliedAPCreditMemos : public XWidget, public Ui::unappliedAPCreditMemos
{
    Q_OBJECT

public:
    unappliedAPCreditMemos(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unappliedAPCreditMemos();
    virtual bool setParams(ParameterList &params);

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sView();
    virtual void sFillList();
    virtual void sApply();

protected slots:
    virtual void languageChange();

};

#endif // UNAPPLIEDAPCREDITMEMOS_H
