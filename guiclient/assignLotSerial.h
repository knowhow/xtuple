/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ASSIGNLOTSERIAL_H
#define ASSIGNLOTSERIAL_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_assignLotSerial.h"

class assignLotSerial : public XDialog, public Ui::assignLotSerial
{
    Q_OBJECT

public:
    assignLotSerial(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~assignLotSerial();

    QPushButton* _print;

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sNew();
    virtual void sDelete();
    virtual void sClose();
    virtual void sAssign();
    virtual void sFillList();
    virtual void sPrint();
    virtual void sPrintOptions();

protected slots:
    virtual void languageChange();

private:
    int _itemlocSeries;
    int _itemlocdistid;
    bool _trapClose;
};

#endif // ASSIGNLOTSERIAL_H
