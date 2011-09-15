/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREATERECURRINGITEMS_H
#define CREATERECURRINGITEMS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_createRecurringItems.h"

class createRecurringItems : public XWidget, public Ui::createRecurringItems
{
    Q_OBJECT

public:
    createRecurringItems(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~createRecurringItems();

public slots:
    virtual void sCreate();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

};

#endif // CREATERECURRINGITEMS_H
