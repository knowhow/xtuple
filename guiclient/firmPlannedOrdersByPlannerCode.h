/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FIRMPLANNEDORDERSBYPLANNERCODE_H
#define FIRMPLANNEDORDERSBYPLANNERCODE_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_firmPlannedOrdersByPlannerCode.h"

class firmPlannedOrdersByPlannerCode : public XDialog, public Ui::firmPlannedOrdersByPlannerCode
{
    Q_OBJECT

public:
    firmPlannedOrdersByPlannerCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~firmPlannedOrdersByPlannerCode();

public slots:
    virtual void sFirm();

protected slots:
    virtual void languageChange();

};

#endif // FIRMPLANNEDORDERSBYPLANNERCODE_H
