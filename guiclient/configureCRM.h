/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGURECRM_H
#define CONFIGURECRM_H

#include "xabstractconfigure.h"

#include "ui_configureCRM.h"

class configureCRM : public XAbstractConfigure, public Ui::configureCRM
{
    Q_OBJECT

public:
    configureCRM(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureCRM();

public slots:
    virtual bool sSave();

protected slots:
    virtual void languageChange();
    virtual void sStrictCountryChanged(bool);

signals:
    void saving();

};

#endif // CONFIGURECRM_H
