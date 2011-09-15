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

#include "guiclient.h"
#include "xwidget.h"
#include "contacts.h"
#include "todoList.h"
#include "opportunityList.h"

#include <QStandardItemModel>
#include "ui_crmaccount.h"

class crmaccount : public XWidget, public Ui::crmaccount
{
    Q_OBJECT

public:
    crmaccount(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~crmaccount();
    static void doDialog(QWidget *, const ParameterList &);
    int	getIncidentId();

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulate();
    virtual void sPopulateRegistrations();

protected slots:
    virtual void languageChange();

    virtual void sClose();
    virtual void sCompetitor();
    virtual void sCustomer();
    virtual void sDeleteCharacteristic();
    virtual void sDeleteReg();
    virtual void sEditCharacteristic();
    virtual void sEditReg();
    virtual void sGetCharacteristics();
    virtual void sNewCharacteristic();
    virtual void sNewReg();
    virtual void sPartner();
    virtual void sProspect();
    virtual void sSave();
    virtual void sTaxAuth();
    virtual void sUpdateRelationships();
    virtual void sEditVendor();
    virtual void sViewVendor();
    virtual void sCustomerToggled();
    virtual void sProspectToggled();
    virtual void sCheckNumber();
    virtual void sHandleButtons();
    virtual void sVendorInfo();
    virtual void sHandleCntctDetach(int cntctId);

protected:
    virtual void closeEvent(QCloseEvent*);
    
    todoList *_todoList;
    contacts *_contacts;
    opportunityList *_oplist;

signals:
    void saved(int);

private:
    bool	_modal;
    int		_mode;
    int		_crmacctId;
    int		_competitorId;
    int		_custId;
    int		_partnerId;
    int		_prospectId;
    int		_taxauthId;
    int		_vendId;
    int		_cntct1Id;
    int		_cntct2Id;
    int         _NumberGen;

    int	saveNoErrorCheck();

};

#endif // CRMACCOUNT_H
