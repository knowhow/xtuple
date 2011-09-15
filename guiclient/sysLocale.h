/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SYSLOCALE_H
#define SYSLOCALE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_sysLocale.h"

class sysLocale : public XDialog, public Ui::sysLocale
{
    Q_OBJECT

public:
    sysLocale(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~sysLocale();

public slots:
    virtual enum SetResponse set( ParameterList & pParams );
    virtual void close();
    virtual void sSave();
    virtual void sUpdateColors();
    virtual void sUpdateSamples();
    virtual void sUpdateCountries();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int     _mode;
    int     _localeid;
    QString convert(const QString &);
    QLocale generateLocale();
};

#endif // SYSLOCALE_H
