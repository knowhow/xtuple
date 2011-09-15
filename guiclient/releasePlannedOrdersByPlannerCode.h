/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RELEASEPLANNEDORDERSBYPLANNERCODE_H
#define RELEASEPLANNEDORDERSBYPLANNERCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_releasePlannedOrdersByPlannerCode.h"

class releasePlannedOrdersByPlannerCode : public XDialog, public Ui::releasePlannedOrdersByPlannerCode
{
    Q_OBJECT

public:
    releasePlannedOrdersByPlannerCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~releasePlannedOrdersByPlannerCode();

public slots:
    virtual void sRelease();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

};

#endif // RELEASEPLANNEDORDERSBYPLANNERCODE_H
