/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_eventManager.h"

class eventManager : public XWidget, public Ui::eventManager
{
    Q_OBJECT

public:
    eventManager(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~eventManager();

public slots:
    virtual void sPopulateMenu(QMenu * menu);
    virtual void sInventoryAvailabilityByWorkOrder();
    virtual void sViewSalesOrder();
    virtual void sViewSalesOrderItem();
    virtual void sPrintPackingList();
    virtual void sIssueCountTag();
    virtual void sViewInventoryHistory();
    virtual void sViewInventoryAvailability();
    virtual void sRecallWo();
    virtual void sChangeWoQty();
    virtual void sChangeWoDueDate();
    virtual void sPrintWoTraveler();
    virtual void sDeleteWorkOrder();
    virtual void sAcknowledge();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );
    virtual void sHandleEventValid(bool);
    virtual void sViewPurchaseOrderItem();
    virtual void sViewTodoItem();
    virtual void sViewIncident();
    virtual void sViewTask();


protected slots:
    virtual void languageChange();

};

#endif // EVENTMANAGER_H
