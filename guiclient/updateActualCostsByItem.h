/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEACTUALCOSTSBYITEM_H
#define UPDATEACTUALCOSTSBYITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_updateActualCostsByItem.h"

class updateActualCostsByItem : public XDialog, public Ui::updateActualCostsByItem
{
    Q_OBJECT

public:
    updateActualCostsByItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~updateActualCostsByItem();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSelectAll();
    virtual void sUpdate();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    bool _updateActual;

};

#endif // UPDATEACTUALCOSTSBYITEM_H
