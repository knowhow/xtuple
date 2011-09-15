/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef LOTSERIAL_H
#define LOTSERIAL_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_lotSerial.h"

class lotSerial : public XDialog, public Ui::lotSerial
{
    Q_OBJECT

public:
    lotSerial(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~lotSerial();

    QPushButton* _print;

public slots:
    virtual void populate();
    virtual void sSave();
    virtual void sChanged();
    virtual void sFillList();
    virtual void sNewCharass();
    virtual void sEditCharass();
    virtual void sDeleteCharass();
    virtual void sNewReg();
    virtual void sEditReg();
    virtual void sDeleteReg();
    virtual void sPrint();

protected slots:
    virtual void languageChange();
    
private:
    int _lsidCache;
    int _itemidCache;
    bool _changed;

};

#endif // LOTSERIAL_H
