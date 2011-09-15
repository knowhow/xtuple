/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COSTCATEGORIES_H
#define COSTCATEGORIES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_costCategories.h"

class costCategories : public XWidget, public Ui::costCategories
{
    Q_OBJECT

public:
    costCategories(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~costCategories();

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * menu );
    virtual void sFillList();
    virtual void sListItemSites();

protected slots:
    virtual void languageChange();

};

#endif // COSTCATEGORIES_H
