/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTCOSTSBYCLASSCODE_H
#define POSTCOSTSBYCLASSCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_postCostsByClassCode.h"

class postCostsByClassCode : public XDialog, public Ui::postCostsByClassCode
{
    Q_OBJECT

public:
    postCostsByClassCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postCostsByClassCode();

public slots:
    virtual void sSelectAll();
    virtual void sPost();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

};

#endif // POSTCOSTSBYCLASSCODE_H
