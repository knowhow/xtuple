/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MAINTAINSHIPPING_H
#define MAINTAINSHIPPING_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_maintainShipping.h"

class maintainShipping : public XWidget, public Ui::maintainShipping
{
    Q_OBJECT

public:
    maintainShipping(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~maintainShipping();

public slots:
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sShippingInformation();
    virtual void sShipOrder();
    virtual void sReturnAllOrderStock();
    virtual void sViewOrder();
    virtual void sPrintShippingForm();
    virtual void sIssueStock();
    virtual void sReturnAllLineStock();
    virtual void sViewLine();
    virtual void sReturnAllStock();
    virtual void sFillList();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

private:
    int _itemtype;

};

#endif // MAINTAINSHIPPING_H
