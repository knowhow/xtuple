/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDOR_H
#define VENDOR_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "contactcluster.h"
#include "ui_vendor.h"

class vendor : public XWidget, public Ui::vendor
{
    Q_OBJECT

public:
    vendor(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~vendor();

    Q_INVOKABLE virtual int id() const;

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sCheck();
    virtual void populate();
    virtual void sPrintAddresses();
    virtual void sNewAddress();
    virtual void sEditAddress();
    virtual void sViewAddress();
    virtual void sDeleteAddress();
    virtual void sFillAddressList();
    virtual void sFillTaxregList();
    virtual void sNewTaxreg();
    virtual void sEditTaxreg();
    virtual void sViewTaxreg();
    virtual void sDeleteTaxreg();
    virtual void sHandleButtons();
    virtual void sLoadCrmAcct(int);
    virtual void sNumberEdited();

    virtual void sNext();
    virtual void sPrevious();

    virtual void clear();

signals:
    void populated();
    void newId(int);
    void saved(int);

protected slots:
    virtual void languageChange();
    virtual bool sCheckSave();

protected:
    virtual void closeEvent(QCloseEvent*);
    QValidator *_accountValidator;
    QValidator *_routingValidator;

private:
    int _mode;
    int _vendid;
    int _crmacctid;
    int _NumberGen;
    QString _cachedNumber;
    bool _ignoreClose;
    bool _notice;

};

#endif // VENDOR_H
