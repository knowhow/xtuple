/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNINVOICEDSHIPMENTS_H
#define UNINVOICEDSHIPMENTS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_uninvoicedShipments.h"

class uninvoicedShipments : public XWidget, public Ui::uninvoicedShipments
{
    Q_OBJECT

public:
    uninvoicedShipments(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~uninvoicedShipments();

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * menu );
    virtual void sSelectForBilling();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // UNINVOICEDSHIPMENTS_H
