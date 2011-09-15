/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDORWORKBENCH_H
#define VENDORWORKBENCH_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_vendorWorkBench.h"

class ParameterList;

//class dspAPApplications;
class dspCheckRegister;
class dspPOsByVendor;
class dspPoItemReceivingsByVendor;
class dspVendorAPHistory;
class unappliedAPCreditMemos;
class selectPayments;

class vendorWorkBench : public XWidget, public Ui::vendorWorkBench
{
    Q_OBJECT

  public:
    vendorWorkBench(QWidget* parent = 0, const char *name = 0, Qt::WFlags f = 0);
    ~vendorWorkBench();

    virtual SetResponse set(const ParameterList &);
    virtual bool        setParams(ParameterList &);

  public slots:
    virtual void clear();
    virtual void sCRMAccount();
    virtual void sPopulate();
    virtual void sPrint();
    virtual void sVendor();
    virtual void sHandleButtons();

  protected slots:
    virtual void languageChange();

  protected:
    //dspAPApplications           *_applications;
    unappliedAPCreditMemos      *_credits;
    selectPayments              *_payables;
    dspPOsByVendor              *_po;
    dspPoItemReceivingsByVendor *_receipts;
    dspVendorAPHistory          *_history;
    dspCheckRegister            *_checks;

  private:
    int _crmacctId;

};

#endif // VENDORWORKBENCH_H
