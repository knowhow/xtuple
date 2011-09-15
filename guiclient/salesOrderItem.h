/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALESORDERITEM_H
#define SALESORDERITEM_H

#include "guiclient.h"
#include <QStandardItemModel>
#include "xdialog.h"
#include <parameter.h>
#include "ui_salesOrderItem.h"

class salesOrderItem : public XDialog, public Ui::salesOrderItem
{
  Q_OBJECT

  public: salesOrderItem(QWidget *parent = 0, const char * = 0, Qt::WindowFlags fl = 0);
    ~salesOrderItem();

    virtual void  prepare();
    virtual void  clear();
    virtual void  setItemExtraClause();

  public slots:
    virtual SetResponse set( const ParameterList &pParams );
    virtual void        sSave();
    virtual void        sPopulateItemsiteInfo();
    virtual void        sListPrices();
    virtual void        sDeterminePrice();
    virtual void        sDeterminePrice( bool force );
    virtual void        sRecalcPrice();
    virtual void        sPopulateItemInfo( int pItemid );
    virtual void        sRecalcAvailability();
    virtual void        sDetermineAvailability();
    virtual void        sDetermineAvailability( bool p );
    virtual void        sCalculateDiscountPrcnt();
    virtual void        sCalculateExtendedPrice();
    virtual void        sHandleWo( bool pCreate );
    virtual void        sPopulateOrderInfo();
    virtual void        sCalculateFromDiscount();
    virtual void        populate();
    virtual void        sFindSellingWarehouseItemsites( int id );
    virtual void        sPriceGroup();
    virtual void        sNext();
    virtual void        sPrev();
    virtual void        sChanged();
    virtual void        sCancel();
    virtual void        sLookupTax();
    virtual void        sTaxDetail();
    virtual void        sQtyUOMChanged();
    virtual void        sPriceUOMChanged();
    virtual void        sCalcWoUnitCost();
    virtual void        sHandleButton();
    virtual void        sHandleScheduleDate();

  protected slots:
    virtual void  languageChange();

    virtual void  reject();

  private:
    QString _custName;
    double  _priceRatio;
    double  _cQtyOrdered;
    QDate   _cScheduledDate;
    int     _preferredWarehouseid;
    int     _shiptoid;
    int     _orderId;
    int     _leadTime;
    int     _custid;
    int     _soheadid;
    int     _soitemid;
    int     _mode;
    int     _initialMode;
    int     _itemsrc;
    bool    _modified;
    bool    _canceling;
    bool    _error;
    int     _availabilityLastItemid;
    int     _availabilityLastWarehousid;
    QDate   _availabilityLastSchedDate;
    bool    _availabilityLastShow;
    bool    _availabilityLastShowIndent;
    double  _originalQtyOrd;
    double  _availabilityQtyOrdered;
    bool    _invIsFractional;
    bool    _updateItemsite;
    double  _orderQtyCache;
    double  _cachedPct;
    double  _cachedRate;
    int     _taxzoneid;
    QStandardItemModel *_itemchar;
    int     _invuomid;
    double  _qtyinvuomratio;
    double  _priceinvuomratio;
    double  _qtyreserved;
    QDate   _dateCache;
    QString _costmethod;

    // For holding variables for characteristic pricing
    QList<QVariant> _charVars;

    enum
    {
      CHAR_ID    = 0,
      CHAR_VALUE = 1,
      CHAR_PRICE = 2
    };

    enum
    {
      ITEM_ID    = 0,
      CUST_ID    = 1,
      SHIPTO_ID  = 2,
      QTY        = 3,
      CURR_ID    = 4,
      EFFECTIVE  = 5
    };
};

#endif  // SALESORDERITEM_H
