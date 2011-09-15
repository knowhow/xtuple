/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FINANCIALLAYOUTCOLUMNS_H
#define FINANCIALLAYOUTCOLUMNS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_financialLayoutColumns.h"

class financialLayoutColumns : public XDialog, public Ui::financialLayoutColumns
{
    Q_OBJECT

public:
    financialLayoutColumns(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~financialLayoutColumns();

    virtual void populate();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sToggleMonth();
    virtual void sToggleQuarter();
    virtual void sToggleYear();
    virtual void sTogglePrior();
    virtual void sToggleYearToDate();
    virtual void sToggleFullYear();
    virtual void sEdit();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _flheadid;
    int _flcolid;

};

#endif // FINANCIALLAYOUTCOLUMNS_H
