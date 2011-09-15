/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WOMATERIALITEM_H
#define WOMATERIALITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_woMaterialItem.h"

class woMaterialItem : public XDialog, public Ui::woMaterialItem
{
    Q_OBJECT

public:
    woMaterialItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~woMaterialItem();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sUpdateQtyRequired();
    virtual void sItemIdChanged();

private:
    int _bomitemid;
    int _mode;
    int _womatlid;
    int _wooperid;
    bool _captive;

};

#endif // WOMATERIALITEM_H
