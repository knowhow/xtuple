/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXAUTHORITY_H
#define TAXAUTHORITY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_taxAuthority.h"

class taxAuthority : public XDialog, public Ui::taxAuthority
{
    Q_OBJECT

public:
    taxAuthority(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~taxAuthority();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();
    virtual void populate();
    virtual void done(int);

protected slots:
    virtual void languageChange();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    int _mode;
    int _taxauthid;
    int _NumberGen;

};

#endif // PRODUCTCATEGORY_H
