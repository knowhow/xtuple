/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRODUCTCATEGORIES_H
#define PRODUCTCATEGORIES_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_productCategories.h"

class productCategories : public XWidget, public Ui::productCategories
{
    Q_OBJECT

public:
    productCategories(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~productCategories();

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList(int pId);
    virtual void sPopulateMenu( QMenu * menu );
    virtual void sPrint();
    virtual void sDeleteUnused();

protected slots:
    virtual void languageChange();

};

#endif // PRODUCTCATEGORIES_H
