/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TERMS_H
#define TERMS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_terms.h"

class terms : public XDialog, public Ui::terms
{
    Q_OBJECT

public:
    terms(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~terms();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual bool sCheck();
    virtual void sSave();
    virtual void sTypeChanged();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _termsid;

};

#endif // TERMS_H
