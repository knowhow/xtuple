/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREATELOTSERIAL_H
#define CREATELOTSERIAL_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_createLotSerial.h"

class createLotSerial : public XDialog, public Ui::createLotSerial
{
    Q_OBJECT

public:
    createLotSerial(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~createLotSerial();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sAssign();
    virtual void sHandleLotSerial();
    virtual void sLotSerialSelected();

protected slots:
    virtual void languageChange();

private:
    bool _fractional;
    int  _itemlocSeries;
    int  _itemlocdistid;
    int  _itemsiteid;
    bool _serial;
    bool _preassigned;
};

#endif // CREATELOTSERIAL_H
