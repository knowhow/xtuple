/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGUREMS_H
#define CONFIGUREMS_H

#include "guiclient.h"
#include "xabstractconfigure.h"

#include "ui_configureMS.h"

class configureMS : public XAbstractConfigure, public Ui::configureMS
{
    Q_OBJECT

public:
    configureMS(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureMS();

public slots:
    virtual bool sSave();

protected slots:
    virtual void languageChange();

signals:
    void saving();
};

#endif // CONFIGUREMS_H
