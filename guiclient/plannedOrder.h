/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PLANNEDORDERS_H
#define PLANNEDORDERS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_plannedOrder.h"

class plannedOrder : public XDialog, public Ui::plannedOrder
{
    Q_OBJECT

public:
    plannedOrder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~plannedOrder();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sClose();
    virtual void sCreate();
    virtual void sUpdateStartDate();
    virtual void sHandleItemsite( int pWarehousid );
    virtual void populate();
    virtual void populateFoNumber();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _planordid;
    bool _captive;

};

#endif // PLANNEDORDERS_H
