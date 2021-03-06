/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPARAPPLICATIONS_H
#define DSPARAPPLICATIONS_H

#include "guiclient.h"

#include "display.h"
#include <QMenu>

#include "ui_dspARApplications.h"

class dspARApplications : public display, public Ui::dspARApplications
{
    Q_OBJECT

public:
    dspARApplications(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*, int);
    virtual void sViewCreditMemo();
    virtual void sViewDebitMemo();
    virtual void sViewInvoice();

protected slots:
    virtual void languageChange();

};

#endif // DSPARAPPLICATIONS_H
