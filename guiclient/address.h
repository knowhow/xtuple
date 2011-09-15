/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ADDRESS_H
#define ADDRESS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include <ui_address.h>
#include "addresscluster.h"

class address : public XDialog, public Ui::address
{
    Q_OBJECT

public:
    address(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~address();

public slots:
    virtual SetResponse set(const ParameterList &pParams);

protected slots:
    virtual void languageChange();

    virtual void sDeleteCharacteristic();
    virtual void sEdit();
    virtual void sEditCharacteristic();
    virtual void sEditContact();
    virtual void sEditShipto();
    virtual void sEditVendor();
    virtual void sEditVendorAddress();
    virtual void sEditWarehouse();
    virtual void sGetCharacteristics();
    virtual void sNewCharacteristic();
    virtual void sPopulate();
    virtual void sPopulateMenu(QMenu *);
    virtual void sSave();
    virtual void sView();
    virtual void sViewContact();
    virtual void sViewShipto();
    virtual void sViewVendor();
    virtual void sViewVendorAddress();
    virtual void sViewWarehouse();

private:
    int _mode;
    int _addrid;
    bool _captive;

    virtual void internalSave(AddressCluster::SaveFlags = AddressCluster::CHECK);
};

#endif // ADDRESS_H
