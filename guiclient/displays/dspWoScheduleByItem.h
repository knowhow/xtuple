/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOSCHEDULEBYITEM_H
#define DSPWOSCHEDULEBYITEM_H

#include "display.h"

#include "ui_dspWoScheduleByItem.h"

class dspWoScheduleByItem : public display, public Ui::dspWoScheduleByItem
{
    Q_OBJECT

public:
    dspWoScheduleByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
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
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * selected, int);
    virtual void sPostProduction();
    virtual void sPrintTraveler();
    virtual void sRecallWO();
    virtual void sReleaseWO();
    virtual void sReprioritizeWo();
    virtual void sRescheduleWO();
    virtual void sView();
    virtual void sViewBOM();
    virtual void sViewWomatl();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOSCHEDULEBYITEM_H
