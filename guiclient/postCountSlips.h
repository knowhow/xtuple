/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTCOUNTSLIPS_H
#define POSTCOUNTSLIPS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_postCountSlips.h"

class postCountSlips : public XDialog, public Ui::postCountSlips
{
    Q_OBJECT

public:
    postCountSlips(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postCountSlips();

public slots:
    virtual void sPost();

protected slots:
    virtual void languageChange();

};

#endif // POSTCOUNTSLIPS_H
