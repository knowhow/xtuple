/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMPRICINGSCHEDULES_H
#define ITEMPRICINGSCHEDULES_H

#include "xwidget.h"

#include "ui_itemPricingSchedules.h"

class itemPricingSchedules : public XWidget, public Ui::itemPricingSchedules
{
    Q_OBJECT

public:
    itemPricingSchedules(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemPricingSchedules();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sDeleteExpired();
    virtual void sFillList();
    virtual void sFillList( int pIpsheadid );
    virtual void sSearch( const QString & );
    virtual void sCopy();

protected slots:
    virtual void languageChange();

};

#endif // ITEMPRICINGSCHEDULES_H
