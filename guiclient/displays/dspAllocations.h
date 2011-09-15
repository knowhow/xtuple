/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPALLOCATIONS_H
#define DSPALLOCATIONS_H

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

#include "ui_dspAllocations.h"

class dspAllocations : public display, public Ui::dspAllocations
{
    Q_OBJECT

public:
    dspAllocations(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sEditCustomerOrder();
    virtual void sEditTransferOrder();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected, int );
    virtual void sViewCustomerOrder();
    virtual void sViewTransferOrder();
    virtual void sViewWorkOrder();

protected slots:
    virtual void languageChange();

};

#endif // DSPALLOCATIONS_H
