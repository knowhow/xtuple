/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ADJUSTINVVALUE_H
#define ADJUSTINVVALUE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_adjustInvValue.h"

class adjustInvValue : public XWidget, public Ui::adjustInvValue
{
    Q_OBJECT

public:
    adjustInvValue(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = Qt::Window);
    ~adjustInvValue();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPost();
    virtual void sPopulate();
    virtual void sUpdateCost();
    virtual void sValidateSite(int siteid);
    virtual void sToggleAltAccnt(bool checked);


protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _itemsiteid;
    double _qtyonhand;
    bool _captive;

};

#endif // ADJUSTINVVALUE_H
