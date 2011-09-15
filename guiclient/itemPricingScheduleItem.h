/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMPRICINGSCHEDULEITEM_H
#define ITEMPRICINGSCHEDULEITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_itemPricingScheduleItem.h"

class itemPricingScheduleItem : public XDialog, public Ui::itemPricingScheduleItem
{
    Q_OBJECT

public:
    itemPricingScheduleItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~itemPricingScheduleItem();

    QPushButton* _save;

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sSave( bool pClose);
    virtual void populate();
    virtual void sUpdateCosts( int pItemid );
    virtual void sUpdateMargins();
    virtual void sTypeChanged(bool);
    virtual void sQtyUOMChanged();
    virtual void sPriceUOMChanged();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sCheckEnable();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _ipsheadid;
    int _ipsitemid;
    int _ipsprodcatid;
    int _ipsfreightid;
    int _invuomid;
    QString _rejectedMsg;

};

#endif // ITEMPRICINGSCHEDULEITEM_H
