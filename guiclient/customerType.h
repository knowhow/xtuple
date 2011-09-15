/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CUSTOMERTYPE_H
#define CUSTOMERTYPE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_customerType.h"

class customerType : public XDialog, public Ui::customerType
{
    Q_OBJECT

public:
    customerType(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~customerType();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _custtypeid;

};

#endif // CUSTOMERTYPE_H
