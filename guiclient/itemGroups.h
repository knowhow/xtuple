/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMGROUPS_H
#define ITEMGROUPS_H

#include "xwidget.h"

#include "ui_itemGroups.h"

class itemGroups : public XWidget, public Ui::itemGroups
{
    Q_OBJECT

public:
    itemGroups(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemGroups();

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList(int pId);

protected slots:
    virtual void languageChange();

};

#endif // ITEMGROUPS_H
