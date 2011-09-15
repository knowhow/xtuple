/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RETURNAUTHITEMLOTSERIAL_H
#define RETURNAUTHITEMLOTSERIAL_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_returnAuthItemLotSerial.h"

class returnAuthItemLotSerial : public XDialog, public Ui::returnAuthItemLotSerial
{
    Q_OBJECT

public:
    returnAuthItemLotSerial(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~returnAuthItemLotSerial();

public slots:
    enum SetResponse set(const ParameterList & pParams );
    void sSave();
    void sCheck();
    void populate();
    void populateItemsite();
    void populateLotSerial();

protected slots:
    void languageChange();
    void closeEvent();

private:
    int _crmacctid;
    int _mode;
    int _raitemid;
    int _raitemlsid;
    int _warehouseid;

};

#endif //  RETURNAUTHITEMLOTSERIAL_H
