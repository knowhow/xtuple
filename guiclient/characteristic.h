/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHARACTERISTIC_H
#define CHARACTERISTIC_H

#include <QSqlTableModel>

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_characteristic.h"

class characteristic : public XDialog, public Ui::characteristic
{
    Q_OBJECT

public:
    characteristic(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~characteristic();

    enum Type { Text, List, Date };

    enum Use { Addresses     = 0x00000001,
               Contacts      = 0x00000002,
               CRMAccounts   = 0x00000004,
               Customers     = 0x00000008,
               Employees     = 0x00000010,
               Incidents     = 0x00000020,
               Items         = 0x00000040,
               LotSerial     = 0x00000080,
               Opportunities = 0x00000100,
             };

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void populate();
    virtual void sFillList();
    virtual void sNew();
    virtual void sDelete();
    virtual void sCharoptClicked(QModelIndex idx);

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCheck();


private:
    int _mode;
    int _charid;
    QSqlTableModel *_charoptModel;
};

#endif // CHARACTERISTIC_H
