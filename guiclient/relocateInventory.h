/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RELOCATEINVENTORY_H
#define RELOCATEINVENTORY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_relocateInventory.h"

class relocateInventory : public XDialog, public Ui::relocateInventory
{
    Q_OBJECT

public:
    relocateInventory(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~relocateInventory();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sMove();
    virtual void sChangeDefaultLocation();
    virtual void sShowHideDefaultToTarget();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

private slots:
    virtual void sFillList();

};

#endif // RELOCATEINVENTORY_H
