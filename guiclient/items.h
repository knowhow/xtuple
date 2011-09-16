/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMS_H
#define ITEMS_H

#include "display.h"
#include <parameter.h>

#include "ui_items.h"

class items : public display, public Ui::items
{
    Q_OBJECT

public:
    items(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sCopy();
    virtual bool setParams(ParameterList &);
};

#endif // ITEMS_H
