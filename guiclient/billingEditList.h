/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BILLINGEDITLIST_H
#define BILLINGEDITLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_billingEditList.h"

class billingEditList : public XWidget, public Ui::billingEditList
{
    Q_OBJECT

public:
    billingEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~billingEditList();
  
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual void sEditBillingOrd();
    virtual void sCancelBillingOrd();
    virtual void sEditBillingQty();
    virtual void sCancelBillingQty();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sFillList();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

private:
    int _orderid;
    int _itemtype;

};

#endif // BILLINGEDITLIST_H
