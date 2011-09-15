/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RECALLORDERS_H
#define RECALLORDERS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_recallOrders.h"

class recallOrders : public XWidget, public Ui::recallOrders
{
    Q_OBJECT

public:
    recallOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~recallOrders();
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual void sRecall();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // RECALLORDERS_H
