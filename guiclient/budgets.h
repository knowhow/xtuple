/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BUDGETS_H
#define BUDGETS_H

#include "xwidget.h"
#include "ui_budgets.h"

class budgets : public XWidget, public Ui::budgets
{
    Q_OBJECT

public:
    budgets(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~budgets();

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPrint();
    virtual void sCopy();

protected slots:
    virtual void languageChange();

};

#endif // BUDGETS_H
