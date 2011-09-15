/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PURGECLOSEDWORKORDERS_H
#define PURGECLOSEDWORKORDERS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_purgeClosedWorkOrders.h"

class purgeClosedWorkOrders : public XDialog, public Ui::purgeClosedWorkOrders
{
    Q_OBJECT

public:
    purgeClosedWorkOrders(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~purgeClosedWorkOrders();

public slots:
    virtual void sPurge();

protected slots:
    virtual void languageChange();

};

#endif // PURGECLOSEDWORKORDERS_H
