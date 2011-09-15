/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FINANCIALLAYOUTGROUP_H
#define FINANCIALLAYOUTGROUP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_financialLayoutGroup.h"

class financialLayoutGroup : public XDialog, public Ui::financialLayoutGroup
{
    Q_OBJECT

public:
    financialLayoutGroup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~financialLayoutGroup();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();
    virtual void populate();
    virtual void sFillGroupList();
    virtual void sToggleShowPrcnt();

protected slots:
    virtual void languageChange();

    virtual void sToggled();


private:
    int _flgrp_flgrpid;
    int _flgrpid;
    int _flheadid;
    int _mode;
    int _rpttype;

};

#endif // FINANCIALLAYOUTGROUP_H
