/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UOM_H
#define UOM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_uom.h"

class uom : public XDialog, public Ui::uom
{
    Q_OBJECT

public:
    uom(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~uom();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sSelected();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCheck();


private:
    int _mode;
    int _uomid;

};

#endif // UOM_H
