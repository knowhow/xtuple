/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREDITMEMOEDITLIST_H
#define CREDITMEMOEDITLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_creditMemoEditList.h"

class creditMemoEditList : public XWidget, public Ui::creditMemoEditList
{
    Q_OBJECT

public:
    creditMemoEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~creditMemoEditList();
  
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual void sEditCreditMemo();
    virtual void sEditCreditMemoItem();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sFillList();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

private:
    int _orderid;
    int _itemtype;

};

#endif // CREDITMEMOEDITLIST_H
