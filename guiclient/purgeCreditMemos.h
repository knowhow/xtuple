/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PURGECREDITMEMOS_H
#define PURGECREDITMEMOS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_purgeCreditMemos.h"

class purgeCreditMemos : public XDialog, public Ui::purgeCreditMemos
{
    Q_OBJECT

public:
    purgeCreditMemos(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~purgeCreditMemos();

public slots:
    virtual void sPurge();

protected slots:
    virtual void languageChange();

};

#endif // PURGECREDITMEMOS_H
