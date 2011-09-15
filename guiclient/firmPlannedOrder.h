/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FIRMPLANNEDORDER_H
#define FIRMPLANNEDORDER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_firmPlannedOrder.h"

class firmPlannedOrder : public XDialog, public Ui::firmPlannedOrder
{
    Q_OBJECT

public:
    firmPlannedOrder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~firmPlannedOrder();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sFirm();

protected slots:
    virtual void languageChange();

private:
    int _planordid;
    int _number;
    int _leadTime;
    int _itemsiteid;
    QString _type;

};

#endif // FIRMPLANNEDORDER_H
