/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCASHRECEIPTS_H
#define DSPCASHRECEIPTS_H

#include "display.h"
#include <parameter.h>

#include "ui_dspCashReceipts.h"

class dspCashReceipts : public display, public Ui::dspCashReceipts
{
    Q_OBJECT

public:
    dspCashReceipts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sEditAropen();
    virtual void sEditCashrcpt();
    virtual void sNew();
    virtual void sNewCashrcpt(); // Legacy code
    virtual void sPostCashrcpt();
    virtual void sViewAropen();
    virtual void sViewCashrcpt();
    virtual void sReversePosted();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
};

#endif // DSPCASHRECEIPTS_H
