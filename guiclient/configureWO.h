/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGUREWO_H
#define CONFIGUREWO_H

#include "xabstractconfigure.h"

#include "ui_configureWO.h"

class configureWO : public XAbstractConfigure, public Ui::configureWO
{
    Q_OBJECT

public:
    configureWO(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureWO();

public slots:
    virtual bool sSave();

protected slots:
    virtual void languageChange();

signals:
    void saving();

};

#endif // CONFIGUREWO_H
