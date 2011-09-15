/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDORPRICELIST_H
#define VENDORPRICELIST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_vendorPriceList.h"

class vendorPriceList : public XDialog, public Ui::vendorPriceList
{
    Q_OBJECT

public:
    vendorPriceList(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~vendorPriceList();
    double _selectedQty;

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sQtyUpdate();
    virtual void sQtyEntered();
    virtual void sPopulatePricing();
    virtual void sSelect();
    virtual void sFillList();


protected slots:
    virtual void languageChange();

private:
  int    _itemsrcid;

};

#endif // VENDORPRICELIST_H
