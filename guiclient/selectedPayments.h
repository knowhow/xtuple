/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SELECTEDPAYMENTS_H
#define SELECTEDPAYMENTS_H

#include "xwidget.h"
#include <parameter.h>
#include "ui_selectedPayments.h"

class selectedPayments : public XWidget, public Ui::selectedPayments
{
    Q_OBJECT

public:
    selectedPayments(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~selectedPayments();

    virtual bool setParams(ParameterList&);

public slots:
    virtual void sEdit();
    virtual void sClear();
    virtual void sFillList(int);
    virtual void sFillList();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // SELECTEDPAYMENTS_H
