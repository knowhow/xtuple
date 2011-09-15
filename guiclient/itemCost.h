/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMCOST_H
#define ITEMCOST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_itemCost.h"

class itemCost : public XDialog, public Ui::itemCost
{
    Q_OBJECT

public:
    itemCost(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~itemCost();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sPopulateCostelem();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    int _itemcostid;
    int _mode;

};

#endif // ITEMCOST_H
