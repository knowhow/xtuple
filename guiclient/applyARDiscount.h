/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef APPLYARDISCOUNT_H
#define APPLYARDISCOUNT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_applyARDiscount.h"

class applyARDiscount : public XDialog, public Ui::applyARDiscount
{
    Q_OBJECT

  public:
    applyARDiscount(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~applyARDiscount();

  public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sApply();

  protected:
    int _aropenid;
    int _cashrcptitemid;

  protected slots:
    virtual void languageChange();
    virtual void populate();
};

#endif // APPLYARDISCOUNT_H
