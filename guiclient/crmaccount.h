/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNT_H
#define CRMACCOUNT_H

#include "contacts.h"
#include "guiclient.h"
#include "todoList.h"
#include "xwidget.h"

#include <QSqlError>
#include "ui_crmaccount.h"

class crmaccount : public XWidget, public Ui::crmaccount
{
    Q_OBJECT

public:
    crmaccount(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~crmaccount();
    static      void doDialog(QWidget *, const ParameterList &);
    Q_INVOKABLE int  id();

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulate();
    virtual void sPopulateRegistrations();
    virtual void setId(int id);

signals:
    int newId(int id);

protected slots:
    virtual void languageChange();

    virtual void sClose();
    virtual void sCompetitor();
    virtual void sCustomer();
    virtual void sDeleteCharacteristic();
    virtual void sDeleteReg();
    virtual void sEditCharacteristic();
    virtual void sEditReg();
    virtual void sEmployee();
    virtual void sGetCharacteristics();
    virtual void sNewCharacteristic();
    virtual void sNewReg();
    virtual void sPartner();
    virtual void sProspect();
    virtual void sSave();
    virtual void sSalesRep();
    virtual void sTaxAuth();
    virtual void sUser();
    virtual void sUpdateRelationships();
    virtual void sEditVendor();
    virtual void sViewVendor();
    virtual void sCustomerToggled();
    virtual void sProspectToggled();
    virtual void sCheckNumber();
    virtual void sHandleButtons();
    virtual void sHandleChildButtons();
    virtual void sVendorInfo();
    virtual void sHandleCntctDetach(int cntctId);

protected:
    virtual void closeEvent(QCloseEvent*);
    
    todoList *_todoList;
    contacts *_contacts;

signals:
    void saved(int);

private:
    bool        _modal;
    int         _mode;
    int         _crmacctId;
    int         _competitorId;
    int         _custId;
    int         _empId;
    int         _partnerId;
    int         _prospectId;
    int         _salesrepId;
    int         _taxauthId;
    QString     _username;
    int         _vendId;
    int         _cntct1Id;
    int         _cntct2Id;
    int         _NumberGen;
    bool        _canCreateUsers;

    QSqlError   saveNoErrorCheck();

};

#endif // CRMACCOUNT_H
