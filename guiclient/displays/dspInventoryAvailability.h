/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINVENTORYAVAILABILITY_H
#define DSPINVENTORYAVAILABILITY_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspInventoryAvailability.h"

#define ITEMSITELEADTIME  		 0
#define LOOKAHEADDAYS			 1
#define CUTOFFDATE			 2
#define DATES     			 3

class dspInventoryAvailability : public display, public Ui::dspInventoryAvailability
{
    Q_OBJECT

public:
    dspInventoryAvailability(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu * menu, QTreeWidgetItem * selected, int);
    virtual void sViewHistory();
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sRunningAvailability();
    virtual void sCreateWO();
    virtual void sPostMiscProduction();
    virtual void sCreatePR();
    virtual void sCreatePO();
    virtual void sViewSubstituteAvailability();
    virtual void sIssueCountTag();
    virtual void sEnterMiscCount();
    virtual void sHandleShowReorder( bool pValue );
    virtual void sByVendorChanged();
    virtual void sAsofChanged(int index);

protected slots:
    virtual void languageChange();

private:
    bool _forgetful;
    QString _settingsName;

};

#endif // DSPINVENTORYAVAILABILITY_H
