/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VOUCHERITEMDISTRIB_H
#define VOUCHERITEMDISTRIB_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_voucherItemDistrib.h"

class voucherItemDistrib : public XDialog, public Ui::voucherItemDistrib
{
    Q_OBJECT

public:
    voucherItemDistrib(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~voucherItemDistrib();

    virtual void init();

public slots:
    virtual SetResponse set( ParameterList & pParams );
    virtual void populate();
    virtual void sSave();
    virtual bool sCheck();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _poitemid;
    int _voheadid;
    int _vodistid;

};

#endif // VOUCHERITEMDISTRIB_H
