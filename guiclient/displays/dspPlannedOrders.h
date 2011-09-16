/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPLANNEDORDERS_H
#define DSPPLANNEDORDERS_H

#include "display.h"

class dspPlannedOrders : public display
{
    Q_OBJECT

public:
    dspPlannedOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sDeleteOrder();
    virtual void sDspRunningAvailability();
    virtual void sDspUsageStatistics();
    virtual void sEditOrder();
    virtual void sFirmOrder();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected, int);
    virtual void sReleaseOrder();
    virtual void sSoftenOrder();
};

#endif // DSPPLANNEDORDERS_H
