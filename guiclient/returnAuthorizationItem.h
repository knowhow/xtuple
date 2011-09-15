/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RETURNAUTHORIZATIONITEM_H
#define RETURNAUTHORIZATIONITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_returnAuthorizationItem.h"

class returnAuthorizationItem : public XDialog, public Ui::returnAuthorizationItem
{
    Q_OBJECT

public:
    returnAuthorizationItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~returnAuthorizationItem();

public slots:
	
    SetResponse set(const ParameterList & pParams );
    void sPopulateItemInfo();
    void sPopulateItemsiteInfo();
    void populate();
    void sCalculateDiscountPrcnt();
    void sCalculateFromDiscount();
    void sCalculateSaleDiscountPrcnt();
    void sCalculateSaleFromDiscount();
    void sListPrices();
    void sSaleListPrices();
    void sCalculateTax();
    void sPriceGroup();
    void sTaxDetail();
    void sQtyUOMChanged();
    void sPriceUOMChanged();
    void sDispositionChanged();
    void sDetermineAvailability();
    void sHandleWo( bool pCreate );
    void sPopulateOrderInfo();
    void sCalcWoUnitCost();
    
    //Lot/Serial page
    void sNew();
    void sEdit();
    void sDelete();
    void sFillList();
   
protected slots:
    void languageChange();

    bool sSave();
    void sSaveClicked();
    void sCalculateExtendedPrice();
    void sCalculateSaleExtendedPrice();
    void updatePriceInfo();
    void rejectEvent();

private:
	
    int      _mode;
    int      _raitemid;
    int	     _raheadid;
    int      _custid;
    int      _crmacctid;
    int	     _coitemid;
    int	     _shiptoid;
    QString  _creditmethod;
    double   _priceRatio;
    double   _listPriceCache;
    double   _salePriceCache;
    double   _qtyAuthCache;
    int	     _taxzoneid;
    double	 _qtySoldCache;
    double   _qtycredited;
	  int	     _dispositionCache;
    int      _invuomid;
    double   _qtyinvuomratio;
    double   _priceinvuomratio;

    int      _orderId;

    int      _availabilityLastItemid;
    int      _availabilityLastWarehousid;
    QDate    _availabilityLastSchedDate;
    bool     _availabilityLastShow;
    double   _availabilityQtyOrdered;
    int      _leadTime;
    QDate    _cScheduledDate;
    double   _cQtyOrdered;
    int      _preferredWarehousid;
    int      _preferredShipWarehousid;
    QString  _status;
    double   _soldQty;
    int      _coitemitemsiteid;
    double   _unitcost;
    int      _origsoid;
    QString  _costmethod;

};

#endif // RETURNAUTHORIZATIONITEM_H
