/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMALIAS_H
#define ITEMALIAS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_itemAlias.h"

class itemAlias : public XDialog, public Ui::itemAlias
{
    Q_OBJECT

public:
    itemAlias(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~itemAlias();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _itemaliasid;
    int _itemid;

};

#endif // ITEMALIAS_H
