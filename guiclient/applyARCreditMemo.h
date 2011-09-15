/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef APPLYARCREDITMEMO_H
#define APPLYARCREDITMEMO_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_applyARCreditMemo.h"

class applyARCreditMemo : public XDialog, public Ui::applyARCreditMemo
{
    Q_OBJECT

public:
    applyARCreditMemo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~applyARCreditMemo();

    virtual void populate();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPost();
    virtual void sApplyBalance();
    virtual void sApplyLineBalance();
    virtual void sApply();
    virtual void sClear();
    virtual void sClose();
    virtual void sPriceGroup();

protected slots:
    virtual void languageChange();

    virtual void sSearchDocNumChanged(const QString &);


private:
    int    _aropenid;
    bool   _captive;

};

#endif // APPLYARCREDITMEMO_H
