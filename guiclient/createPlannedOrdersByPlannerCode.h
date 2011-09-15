/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREATEPLANNEDORDERSBYPLANNERCODE_H
#define CREATEPLANNEDORDERSBYPLANNERCODE_H

#include <QProgressDialog>

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_createPlannedOrdersByPlannerCode.h"

class createPlannedOrdersByPlannerCode : public XDialog, public Ui::createPlannedOrdersByPlannerCode
{
    Q_OBJECT

public:
    createPlannedOrdersByPlannerCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~createPlannedOrdersByPlannerCode();
    virtual bool setParams(ParameterList&);

public slots:
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

    virtual void sCreate();
    virtual void sCreate(ParameterList params);
};

#endif // CREATEPLANNEDORDERSBYPLANNERCODE_H
