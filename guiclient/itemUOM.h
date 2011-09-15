/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMUOM_H
#define ITEMUOM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_itemUOM.h"

class itemUOM : public XDialog, public Ui::itemUOM
{
    Q_OBJECT

public:
    itemUOM(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~itemUOM();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sFillList();
    virtual void reject();
    virtual void sAdd();
    virtual void sRemove();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCheck();

private:
    int _mode;
    int _itemid;
    int _itemuomconvid;
    bool _ignoreSignals;
    int _uomidFrom;

};

#endif // ITEMUOM_H
