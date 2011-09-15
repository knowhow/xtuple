/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIPPINGZONE_H
#define SHIPPINGZONE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_shippingZone.h"

class shippingZone : public XDialog, public Ui::shippingZone
{
    Q_OBJECT

public:
    shippingZone(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~shippingZone();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCheck();


private:
    int _mode;
    int _shipzoneid;

};

#endif // SHIPPINGZONE_H
