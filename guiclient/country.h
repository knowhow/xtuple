/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COUNTRY_H
#define COUNTRY_H

#include "guiclient.h"
#include "xdialog.h"

#include "parameter.h"

#include "ui_country.h"

class country : public XDialog, public Ui::country
{
    Q_OBJECT

public:
    country(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~country();

    virtual SetResponse set(const ParameterList & pParams );

public slots:
    virtual void populate();
    virtual void sSave();
    virtual void sToUpper();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _countryId;

private slots:
};

#endif // COUNTRY
