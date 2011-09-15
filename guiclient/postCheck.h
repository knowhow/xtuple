/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTCHECK_H
#define POSTCHECK_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_postCheck.h"

class postCheck : public XDialog, public Ui::postCheck
{
    Q_OBJECT

public:
    postCheck(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postCheck();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPost();
    virtual void sHandleBankAccount( int pBankaccntid );
    virtual void populate( int pApchkid );

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // POSTCHECK_H
