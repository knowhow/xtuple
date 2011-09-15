/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PURCHASEORDER_H
#define PURCHASEORDER_H

#include "guiclient.h"
#include "xwidget.h"

#include <parameter.h>

#include "ui_purchaseOrder.h"

class PoitemTableModel;

class purchaseOrder : public XWidget, public Ui::purchaseOrder
{
    Q_OBJECT

public:
    purchaseOrder(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~purchaseOrder();

    virtual void createHeader();
    virtual void populateOrderNumber();
	virtual void saveDetail();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sSave();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sVendaddrList();
    virtual void sHandleDeleteButton();
    virtual void sHandleOrderDate();
    virtual void sHandleVendor( int pVendid );
    virtual void sFillList();
    virtual void sCalculateTotals();
    virtual void sSetUserOrderNumber();
    virtual void sHandleOrderNumber();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sCalculateTax();
    virtual void sTaxZoneChanged();
    virtual void sTaxDetail();
    virtual void sViewSo();
    virtual void sViewWo();
    virtual void sEditSo();
    virtual void sEditWo();
	virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
	virtual void sHandleShipTo();

protected slots:
    virtual void languageChange();
    virtual void sCurrencyChanged();
    virtual void sQEDelete();
    virtual bool sQESave();
    virtual void sTabChanged(int);

signals:
   void saved(int);

private:
    void setPoheadid(const int);
    bool _captive;
    bool _userOrderNumber;
    bool _useWarehouseFOB;
    int	_cachedTabIndex;
    int _deleteMode;
    int _vendaddrid;
    int _poheadid;
    QList<int> _pridList;
    int _mode;
    bool _printed;
    PoitemTableModel* _qeitem;
};

#endif // PURCHASEORDER_H
