/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMPRICINGSCHEDULE_H
#define ITEMPRICINGSCHEDULE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_itemPricingSchedule.h"

class itemPricingSchedule : public XDialog, public Ui::itemPricingSchedule
{
    Q_OBJECT

public:
    itemPricingSchedule(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~itemPricingSchedule();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual bool sSave(bool p);
    virtual void sCheck();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sFillList( int pIpsitemid );
    virtual void populate();
    virtual void reject();

protected slots:
    virtual void languageChange();

private:
    int _ipsheadid;
    int _mode;
    QDate _updated;
    QString _rejectedMsg;

};

#endif // ITEMPRICINGSCHEDULE_H
