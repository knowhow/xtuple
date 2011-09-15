/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PACKINGLISTBATCH_H
#define PACKINGLISTBATCH_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_packingListBatch.h"

class packingListBatch : public XWidget, public Ui::packingListBatch
{
    Q_OBJECT

public:
    packingListBatch(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~packingListBatch();

public slots:
    virtual void sAddSO();
    virtual void sAddTO();
    virtual void sClearPrinted();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );
    virtual void sPopulateMenu( QMenu * );
    virtual void sPrintBatch();
    virtual void sPrintEditList();
    virtual void sPrintPackingList();
    virtual void sViewSalesOrder();
    virtual void sViewTransferOrder();
    virtual void setParams(ParameterList &);

protected slots:
    virtual void languageChange();

};

#endif // PACKINGLISTBATCH_H
