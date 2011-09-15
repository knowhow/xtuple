/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIPPINGCHARGETYPES_H
#define SHIPPINGCHARGETYPES_H

#include "xwidget.h"
#include "ui_shippingChargeTypes.h"

class shippingChargeTypes : public XWidget, public Ui::shippingChargeTypes
{
    Q_OBJECT

public:
    shippingChargeTypes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~shippingChargeTypes();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // SHIPPINGCHARGETYPES_H
