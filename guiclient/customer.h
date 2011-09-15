/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "guiclient.h"
#include "xwidget.h"

#include "contacts.h"
#include "todoList.h"
#include "opportunityList.h"
#include "quotes.h"
#include "openSalesOrders.h"
#include "returnAuthorizationWorkbench.h"
#include "dspAROpenItems.h"
#include "dspCashReceipts.h"
#include "dspCreditCardTransactions.h"

#include <QStandardItemModel>
#include <parameter.h>

#include "ui_customer.h"

class customer : public XWidget, public Ui::customer
{
    Q_OBJECT

public:
    customer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~customer();

    Q_INVOKABLE virtual int id()   const;
    Q_INVOKABLE virtual int mode() const;

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void setId(int);
    virtual void currentTabChanged(int index);
    virtual void populate();
    virtual void sCancel();
    virtual void sCheck();
    virtual bool sCheckRequired();
    virtual void sDeleteCharacteristic();
    virtual void sDeleteShipto();
    virtual void sDeleteTaxreg();
    virtual void sEditCharacteristic();
    virtual void sEditCreditCard();
    virtual void sEditShipto();
    virtual void sEditTaxreg();
    virtual void sFillList();
    virtual void sFillCcardList();
    virtual void sFillCharacteristicList();
    virtual void sFillShiptoList();
    virtual void sFillTaxregList();
    virtual void sMoveDown();
    virtual void sMoveUp();
    virtual void sNewCharacteristic();
    virtual void sNewCreditCard();
    virtual void sNewShipto();
    virtual void sNewTaxreg();
    virtual void sPopulateCommission();
    virtual void sPopulateShiptoMenu( QMenu * menuThis );
    virtual void sPopulateSummary();
    virtual void sPrint();
    virtual void sPrintShipto();
    virtual bool sSave();
    virtual void sSaveClicked();
    virtual void sViewCreditCard();
    virtual void sViewShipto();
    virtual void sViewTaxreg();
    virtual void sLoadProspect(int);
    virtual void sLoadCrmAcct(int);
    virtual void sHandleButtons();
    virtual void sClear();
    virtual void sNumberEditable(bool);
    virtual void sPrepare();

protected slots:
    virtual void languageChange();
    virtual void sNumberEdited();

signals:
    void populated();
    void newId(int);
    void newMode(int);
    void saved(int);

protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void setValid(bool valid);
    todoList *_todoList;
    contacts *_contacts;
    opportunityList *_oplist;
    quotes *_quotes;
    openSalesOrders *_orders;
    returnAuthorizationWorkbench *_returns;
    dspAROpenItems *_aritems;
    dspCashReceipts *_cashreceipts;
    dspCreditCardTransactions *_cctrans;

private:
    int _mode;
    int _custid;
    int	_crmacctid;
    int _NumberGen;
    QString _cachedNumber;
    QString key;
    bool _notice;
    bool _autoSaved;
    bool _captive;
    bool _charfilled;
    QStandardItemModel * _custchar;

};

#endif // CUSTOMER_H
