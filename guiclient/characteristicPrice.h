/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHARACTERISTICPRICE_H
#define CHARACTERISTICPRICE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_characteristicPrice.h"

class characteristicPrice : public XDialog, public Ui::characteristicPrice
{
    Q_OBJECT

public:
    characteristicPrice(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~characteristicPrice();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();
    virtual void populateItemcharInfo();
    virtual void sCheck();
    virtual void sCharIdChanged();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _itemid;
    int _ipsitemid;
    int _ipsitemcharid;
    QString _rejectedMsg;

};

#endif // CHARACTERISTICPRICE_H
