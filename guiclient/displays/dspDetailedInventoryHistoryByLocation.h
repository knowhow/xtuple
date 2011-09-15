/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPDETAILEDINVENTORYHISTORYBYLOCATION_H
#define DSPDETAILEDINVENTORYHISTORYBYLOCATION_H

#include "display.h"

#include "ui_dspDetailedInventoryHistoryByLocation.h"

class dspDetailedInventoryHistoryByLocation : public display, public Ui::dspDetailedInventoryHistoryByLocation
{
    Q_OBJECT

public:
    dspDetailedInventoryHistoryByLocation(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPopulateLocations();
    virtual void sPopulateLocationInfo( int pLocationid );
    virtual void sViewTransInfo();
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int);

protected slots:
    virtual void languageChange();

};

#endif // DSPDETAILEDINVENTORYHISTORYBYLOCATION_H
