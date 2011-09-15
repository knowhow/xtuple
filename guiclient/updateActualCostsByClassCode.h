/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEACTUALCOSTSBYCLASSCODE_H
#define UPDATEACTUALCOSTSBYCLASSCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_updateActualCostsByClassCode.h"

class updateActualCostsByClassCode : public XDialog, public Ui::updateActualCostsByClassCode
{
    Q_OBJECT

public:
    updateActualCostsByClassCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~updateActualCostsByClassCode();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSelectAll();
    virtual void sUpdate();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    bool _updateActual;

};

#endif // UPDATEACTUALCOSTSBYCLASSCODE_H
