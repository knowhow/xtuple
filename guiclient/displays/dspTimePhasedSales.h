/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDSALES_H
#define DSPTIMEPHASEDSALES_H

#include "displayTimePhased.h"

#include "ui_dspTimePhasedBookings.h"

class dspTimePhasedSales : public displayTimePhased, public Ui::dspTimePhasedBookings
{
    Q_OBJECT

public:
    dspTimePhasedSales(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParamsTP(ParameterList &);

public slots:
    virtual void sGroupByChanged();
    virtual void sViewHistory();
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem *, int pColumn);

protected slots:
    virtual void languageChange();
};

#endif // DSPTIMEPHASEDSALES_H
