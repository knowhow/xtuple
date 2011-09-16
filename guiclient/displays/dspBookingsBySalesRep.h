/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPBOOKINGSBYSALESREP_H
#define DSPBOOKINGSBYSALESREP_H

#include "display.h"

#include "ui_dspBookingsBySalesRep.h"

class dspBookingsBySalesRep : public display, public Ui::dspBookingsBySalesRep
{
    Q_OBJECT

public:
    dspBookingsBySalesRep(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &params);

protected slots:
    virtual void languageChange();

};

#endif // DSPBOOKINGSBYSALESREP_H
