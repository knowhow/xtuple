/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RETURNWOMATERIALITEM_H
#define RETURNWOMATERIALITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_returnWoMaterialItem.h"

class returnWoMaterialItem : public XDialog, public Ui::returnWoMaterialItem
{
    Q_OBJECT

public:
    returnWoMaterialItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~returnWoMaterialItem();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sReturn();
    virtual void sSetQOH( int pWomatlid );
    virtual void sUpdateQty();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    double _cachedQOH;

};

#endif // RETURNWOMATERIALITEM_H
