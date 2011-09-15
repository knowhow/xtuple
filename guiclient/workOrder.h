/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WORKORDER_H
#define WORKORDER_H

#include "guiclient.h"
#include "xwidget.h"
#include <QStandardItemModel>
#include <parameter.h>
#include "ui_workOrder.h"

class workOrder : public XWidget, public Ui::workOrder
{
    Q_OBJECT

public:
    workOrder(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~workOrder();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCreate();
    virtual void sUpdateStartDate();
    virtual void sPopulateLeadTime( int pWarehousid );
    virtual void sPopulateItemChar( int pItemid );
    virtual void populate();
    virtual void populateWoNumber();
    virtual void sClose();
    virtual void sHandleButtons();
    virtual void sFillList();
    virtual void sPostProduction();
    virtual void sCorrectProductionPosting();
    virtual void sRecallWO();
    virtual void sReleaseWO();
    virtual void sExplodeWO();
    virtual void sImplodeWO();
    virtual void sDeleteWO();
    virtual void sCloseWO();
    virtual void sChangeWOQty();
    virtual void sChangeParentQty();
    virtual void sInventoryAvailabilityByWorkOrder();
    virtual void sPrintTraveler();
    virtual void sReprioritizeWo();
    virtual void sReprioritizeParent();
    virtual void sRescheduleWO();
    virtual void sRescheduleParent();
    virtual void sDspRunningAvailability();
    virtual void sIssueMatlBatch();
    virtual void sReturnMatlBatch();
    virtual void sIssueMatl();
    virtual void sReturnMatl();
    virtual void sScrapMatl();
    virtual void sNewMatl();
    virtual void sEditMatl();
    virtual void sViewMatl();
    virtual void sDeleteMatl();
    virtual void sViewMatlAvailability();
    virtual void sViewMatlSubstituteAvailability();
    virtual void sSubstituteMatl();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sSave();

protected slots:
    virtual void languageChange();

signals:
    void saved(int);

private:
    bool _captive;
    int _planordid;
    int _sense;
    int _sourceid;
    QString _planordtype;
    int _mode;
    int _lastWarehousid;
    int _lastItemid;
    QStandardItemModel * _itemchar;
    int _woid;
    int _wonumber;

    int _oldPriority;
    QDate _oldStartDate;
    QDate _oldDueDate;
    double _oldQty;

};

#endif // WORKORDER_H
