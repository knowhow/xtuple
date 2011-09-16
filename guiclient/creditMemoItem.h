/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREDITMEMOITEM_H
#define CREDITMEMOITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_creditMemoItem.h"

class creditMemoItem : public XDialog, public Ui::creditMemoItem
{
    Q_OBJECT

public:
    creditMemoItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~creditMemoItem();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sPopulateItemInfo();
    virtual void sPopulateItemsiteInfo();
    virtual void populate();
    virtual void sCalculateDiscountPrcnt();
    virtual void sCalculateFromDiscount();
    virtual void sListPrices();
    virtual void sCalculateTax();
    virtual void sPriceGroup();
    virtual void sTaxDetail();
    virtual void sQtyUOMChanged();
    virtual void sPriceUOMChanged();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCalculateExtendedPrice();
    virtual void updatePriceInfo();

private:
    int _mode;
    int _cmitemid;
    int _cmheadid;
    int _custid;
    int _invoiceNumber;
    int _shiptoid;
    double _priceRatio;
    double _listPriceCache;
    double _salePriceCache;
    double _qtyShippedCache;
    int		_taxzoneid;
    int _invuomid;
    double _qtyinvuomratio;
    double _priceinvuomratio;
    double _ratio;
    bool _saved;

};

#endif // CREDITMEMOITEM_H
