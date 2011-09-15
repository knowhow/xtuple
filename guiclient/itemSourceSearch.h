/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSOURCESEARCH_H
#define ITEMSOURCESEARCH_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_itemSourceSearch.h"

class itemSourceSearch : public XDialog, public Ui::itemSourceSearch
{
    Q_OBJECT

public:
    itemSourceSearch(QWidget* parent = 0, const char* name = 0, bool model = false, Qt::WFlags fl = 0);
    ~itemSourceSearch();

    int itemsrcId();
    int expcatId();
    QString vendItemNumber();
    QString vendItemDescrip();
    QString manufName();
    QString manufItemNumber();
    QString manufItemDescrip();

public slots:
    virtual enum SetResponse set(const ParameterList &);

protected slots:
    virtual void languageChange();

    virtual void sFillList();

private:
    int _itemid;
    int _vendid;
};

#endif // ITEMSOURCESEARCH_H
