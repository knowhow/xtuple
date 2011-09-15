/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSFERORDERS_H
#define TRANSFERORDERS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_transferOrders.h"

class transferOrders : public XWidget, public Ui::transferOrders
{
    Q_OBJECT

public:
    transferOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~transferOrders();

public slots:
    virtual void sAddToPackingListBatch();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sIssue();
    virtual void sNew();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sPrint();
    virtual void sPrintPackingList();
    virtual void sRelease();
    virtual void sView();
    virtual void setParams(ParameterList &);

protected slots:
    virtual void languageChange();

private:
    int _numSelected;

};

#endif // TRANSFERORDERS_H
