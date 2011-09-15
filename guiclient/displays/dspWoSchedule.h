/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOSCHEDULE_H
#define DSPWOSCHEDULE_H

#include "guiclient.h"
#include "display.h"

class dspWoSchedule : public display
{
    Q_OBJECT

public:
    dspWoSchedule(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sChangeWOQty();
    virtual void sCloseWO();
    virtual void sCorrectProductionPosting();
    virtual void sDeleteWO();
    virtual void sDspRunningAvailability();
    virtual void sEdit();
    virtual void sExplodeWO();
    virtual void sImplodeWO();
    virtual void sInventoryAvailabilityByWorkOrder();
    virtual void sIssueWoMaterialItem();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected, int);
    virtual void sPostProduction();
    virtual void sPrintTraveler();
    virtual void sRecallWO();
    virtual void sReleaseWO();
    virtual void sReprioritizeWo();
    virtual void sRescheduleWO();
    virtual void sView();
    virtual void sViewBOM();
    virtual void sViewParentSO();
    virtual void sViewParentWO();
    virtual void sViewWomatl();
    virtual void sNew();
};

#endif // DSPWOSCHEDULE_H
