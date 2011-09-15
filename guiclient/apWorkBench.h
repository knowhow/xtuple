/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef APWORKBENCH_H
#define APWORKBENCH_H

#include "guiclient.h"

#include "xwidget.h"
#include <QMenu>

#include <parameter.h>

#include "ui_apWorkBench.h"

class openVouchers;
class selectPayments;
class selectedPayments;
class unappliedAPCreditMemos;
class viewCheckRun;

#include "vendorgroup.h"

class apWorkBench : public XWidget, public Ui::apWorkBench
{
    Q_OBJECT

  public:
    apWorkBench(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~apWorkBench();

    virtual SetResponse set(const ParameterList & pParams);

  protected slots:
    virtual void languageChange();

  protected:
    viewCheckRun           *_checkRun;
    unappliedAPCreditMemos *_credits;
    selectPayments         *_payables;
    openVouchers           *_vouchers;
    selectedPayments       *_selectedPayments;
};

#endif // APWORKBENCH_H
