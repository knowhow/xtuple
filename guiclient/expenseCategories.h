/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EXPENSECATEGORIES_H
#define EXPENSECATEGORIES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_expenseCategories.h"

class expenseCategories : public XWidget, public Ui::expenseCategories
{
    Q_OBJECT

public:
    expenseCategories(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~expenseCategories();

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sCopy();
    virtual void sView();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // EXPENSECATEGORIES_H
