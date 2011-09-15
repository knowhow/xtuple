/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDORADDRESSLIST_H
#define VENDORADDRESSLIST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_vendorAddressList.h"

class vendorAddressList : public XDialog, public Ui::vendorAddressList
{
    Q_OBJECT

public:
    vendorAddressList(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~vendorAddressList();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sClose();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

    virtual void sSelect();


private:
    int _vendid;
    int _vendaddrid;

};

#endif // VENDORADDRESSLIST_H
