/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SUBACCOUNTS_H
#define SUBACCOUNTS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_subaccounts.h"

class subaccounts : public XWidget, public Ui::subaccounts
{
    Q_OBJECT

public:
    subaccounts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~subaccounts();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // SUBACCOUNTS_H
