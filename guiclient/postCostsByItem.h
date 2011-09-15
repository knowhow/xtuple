/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTCOSTSBYITEM_H
#define POSTCOSTSBYITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_postCostsByItem.h"

class postCostsByItem : public XDialog, public Ui::postCostsByItem
{
    Q_OBJECT

public:
    postCostsByItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postCostsByItem();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSelectAll();
    virtual void sPost();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // POSTCOSTSBYITEM_H
