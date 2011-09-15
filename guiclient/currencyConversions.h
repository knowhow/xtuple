/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CURRENCYCONVERSIONS_H
#define CURRENCYCONVERSIONS_H

#include "guiclient.h"
#include "xwidget.h"
#include "parameter.h"

#include "ui_currencyConversions.h"

class currencyConversions : public XWidget, public Ui::currencyConversions
{
    Q_OBJECT

public:
    currencyConversions(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~currencyConversions();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sPrint();
    virtual void sClose();
    virtual void sDelete();
    virtual void sNewCurrency();
    virtual void sFillList();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void setBaseCurrency();
    virtual bool setParams(ParameterList &);

protected slots:
    virtual void languageChange();

private:

};

#endif // CURRENCYCONVERSIONS_H
