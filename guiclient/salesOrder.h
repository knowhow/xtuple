/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALESORDER_H
#define SALESORDER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_salesOrder.h"

class salesOrder : public XWidget, public Ui::salesOrder
{
  Q_OBJECT

  public: salesOrder(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = Qt::Window);
    ~salesOrder();

    Q_INVOKABLE virtual bool  save( bool partial );
    Q_INVOKABLE virtual void  setViewMode();
    Q_INVOKABLE static void   newSalesOrder( int pCustid, QWidget *parent = 0 );
    Q_INVOKABLE static void   editSalesOrder( int pId, bool enableSaveAndAdd, QWidget *parent = 0 );
    Q_INVOKABLE static void   viewSalesOrder( int pId, QWidget *parent = 0 );

  public slots:
    virtual SetResponse set(const ParameterList &pParams );
    virtual void        sSave();
    virtual void        sSaveAndAdd();
    virtual void        sPopulateMenu(QMenu *pMenu);
    virtual void        populateOrderNumber();
    virtual void        sSetUserEnteredOrderNumber();
    virtual void        sHandleOrderNumber();
    virtual void        sPopulateFOB( int pWarehousid );
    virtual void        sPopulateCustomerInfo( int pCustid );
    virtual void        sParseShipToNumber();
    virtual void        populateShipto( int pShiptoid );
    virtual void        sConvertShipTo();
    virtual void        sNew();
    virtual void        sCopyToShipto();
    virtual void        sEdit();
    virtual void        sHandleButtons();
    virtual void        sFirm();
    virtual void        sSoften();
    virtual void        sAction();
    virtual void        sDelete();
    virtual void        populate();
    virtual void        sFillItemList();
    virtual void        sCalculateTotal();
    virtual void        sClear();
    virtual void        clear();
    virtual void        closeEvent( QCloseEvent *pEvent );
    virtual void        sFreightChanged();
    virtual void        sHandleShipchrg( int pShipchrgid );
    virtual void        sHandleSalesOrderEvent( int pSoheadid, bool );
    virtual void        sTaxZoneChanged();
    virtual void        sTaxDetail();
    virtual void        sFreightDetail();
    virtual void        setFreeFormShipto( bool pFreeForm );
    virtual void        populateCMInfo();
    virtual void        populateCCInfo();
    virtual void        sNewCreditCard();
    virtual void        sEditCreditCard();
    virtual void        sViewCreditCard();
    virtual void        sMoveUp();
    virtual void        sMoveDown();
    virtual void        sFillCcardList();
    virtual void        sAuthorizeCC();
    virtual void        sChargeCC();
    virtual void        sReturnStock();
    virtual void        sIssueStock();
    virtual void        sIssueLineBalance();
    virtual void        sReserveStock();
    virtual void        sReserveLineBalance();
    virtual void        sUnreserveStock();
    virtual void        sShowReservations();
    virtual void        sAllocateCreditMemos();
    virtual void        sCheckValidContacts();
    virtual void        sHandleMore();
    virtual void        sCalculateTax();
    virtual void        sRecalculatePrice();
    virtual void        sOrderDateChanged();
    virtual void        sShipDateChanged();
    virtual void        sViewWO();
    virtual void        sMaintainWO();
    virtual void        sViewPO();
    virtual void        sMaintainPO();
    virtual void        sReleasePR();
    virtual void        sViewPR();

  protected slots:
    virtual void  languageChange();
    virtual bool  okToProcessCC();

  signals:
    void saved(int);

  private:
    bool    deleteForCancel();

    bool    _saved;
    bool    _calcfreight;
    int     _orderNumberGen;
    double  _freightCache;
    double  _amountOutstanding;
    double  _amountAllocated;
    double  _taxableSubtotal;
    bool    _userEnteredOrderNumber;
    bool    _ignoreSignals;
    bool    _blanketPos;
    bool    _usesPos;
    bool    _ffShipto;
    bool    _captive;
    int     _soheadid;
    int     _lineMode;
    int     _mode;
    int     _numSelected;
    int     _originalPrjid;
    int     _custtaxzoneid;
    int     _taxzoneidCache;
    int     _crmacctid;
    QDate   _orderDateCache;
    QDate   _shipDateCache;
    bool    _locked;
};

#endif  // SALESORDER_H
