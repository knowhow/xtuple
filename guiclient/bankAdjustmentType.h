/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BANKADJUSTMENTTYPE_H
#define BANKADJUSTMENTTYPE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_bankAdjustmentType.h"

class bankAdjustmentType : public XDialog, public Ui::bankAdjustmentType
{
    Q_OBJECT

public:
    bankAdjustmentType(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~bankAdjustmentType();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sSave();


private:
    int _bankadjtypeid;
    int _mode;

};

#endif // BANKADJUSTMENTTYPE_H
