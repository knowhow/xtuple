/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMAVAILABILITYWORKBENCH_H
#define ITEMAVAILABILITYWORKBENCH_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_itemAvailabilityWorkbench.h"

class itemAvailabilityWorkbench : public XWidget, public Ui::itemAvailabilityWorkbench
{
    Q_OBJECT

public:
    itemAvailabilityWorkbench(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemAvailabilityWorkbench();

    virtual bool setParamsCosted(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void setParams(ParameterList & params);
    virtual void sFillListWhereUsed();
    virtual void sFillListItemloc();
    virtual void sFillListInvhist();
    virtual void sFillListCosted();
    virtual void sFillListRunning();
    virtual void sHandleResort();
    virtual void sHandleShowReorder( bool pValue );
    virtual void sFillListAvail();
    virtual void sPrintRunning();
    virtual void sPrintAvail();
    virtual void sPrintCosted();
    virtual void sPrintHistory();
    virtual void sPrintLocation();
    virtual void sPrintWhereUsed();
    virtual void sPopulateMenuRunning( QMenu*, QTreeWidgetItem* );
    virtual void sPopulateMenuAvail( QMenu*, QTreeWidgetItem* );
    virtual void sPopulateMenuCosted( QMenu*, QTreeWidgetItem* );
    virtual void sPopulateMenuHistory( QMenu*, QTreeWidgetItem* );
    virtual void sPopulateMenuLocation( QMenu*, QTreeWidgetItem* );
    virtual void sPopulateMenuWhereUsed( QMenu*, QTreeWidgetItem* );
    virtual void sViewHistory();
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sRunningAvailability();
    virtual void sCreatePR();
    virtual void sCreatePO();
    virtual void sCreateWO();
    virtual void sIssueCountTag();
    virtual void sEnterMiscCount();
    virtual void sViewSubstituteAvailability();
    virtual void sSoftenOrder();
    virtual void sFirmOrder();
    virtual void sReleaseOrder();
    virtual void sDeleteOrder();
    virtual void sMaintainItemCosts();
    virtual void sViewItemCosting();
    virtual void sViewTransInfo();
    virtual void sEditTransInfo();
    virtual void sViewWOInfo();
    virtual void sViewWOInfoHistory();
    virtual void sRelocateInventory();
    virtual void sReassignLotSerial();
    virtual void sEditBOM();
    virtual void sEditItem();
    virtual void sViewInventoryHistory();
    virtual void sClearQueries();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _costsGroupInt;

};

#endif // ITEMAVAILABILITYWORKBENCH_H
