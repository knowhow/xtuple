/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WAREHOUSES_H
#define WAREHOUSES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_warehouses.h"

class warehouses : public XWidget, public Ui::warehouses
{
    Q_OBJECT

public:
    warehouses(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~warehouses();

public slots:
    virtual void sNew();
    virtual void sView();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sListItemSites();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sPrint();
    virtual void setParams(ParameterList &);

protected slots:
    virtual void languageChange();

};

#endif // WAREHOUSES_H
