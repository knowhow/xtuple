/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PREPARECHECKRUN_H
#define PREPARECHECKRUN_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_prepareCheckRun.h"

class prepareCheckRun : public XDialog, public Ui::prepareCheckRun
{
    Q_OBJECT

public:
    prepareCheckRun(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~prepareCheckRun();

public slots:
    enum SetResponse set(const ParameterList &pParams);
    virtual void sPrint();
    
protected slots:
    virtual void languageChange();

};

#endif // PREPARECHECKRUN_H
