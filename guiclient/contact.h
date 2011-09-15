/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONTACT_H
#define CONTACT_H

#include "guiclient.h"
#include "xdialog.h"
#include <QMenu>

#include <parameter.h>
#include <ui_contact.h>
#include "addresscluster.h"
#include "contactcluster.h"

class contact : public XDialog, public Ui::contact
{
    Q_OBJECT

public:
    contact(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~contact();

    QPushButton *_save;

public slots:
    virtual SetResponse set(const ParameterList &pParams);

protected slots:
    virtual void languageChange();

    virtual void sClose();
    virtual void sDeleteCharass();
    virtual void sDetachUse();
    virtual void sEditCharass();
    virtual void sEditCRMAccount();
    virtual void sEditCustomer();
    virtual void sEditEmployee();
    virtual void sEditProspect();
    virtual void sEditShipto();
    virtual void sEditUse();
    virtual void sEditVendorAddress();
    virtual void sEditVendor();
    virtual void sEditWarehouse();
    virtual void sFillList();
    virtual void sHandleValidUse(bool);
    virtual void sNewCharass();
    virtual void sPopulate();
    virtual void sPopulateUsesMenu(QMenu*);
    virtual void sSave();
    virtual void sViewCRMAccount();
    virtual void sViewCustomer();
    virtual void sViewEmployee();
    virtual void sViewProspect();
    virtual void sViewShipto();
    virtual void sViewUse();
    virtual void sViewVendorAddress();
    virtual void sViewVendor();
    virtual void sViewWarehouse();

private:
    bool _activeCache;
    int  _addrid;
    bool _captive;
    int  _mode;

};

#endif // CONTACT_H
