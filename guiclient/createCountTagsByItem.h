/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREATECOUNTTAGSBYITEM_H
#define CREATECOUNTTAGSBYITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_createCountTagsByItem.h"

class createCountTagsByItem : public XDialog, public Ui::createCountTagsByItem
{
    Q_OBJECT

public:
    createCountTagsByItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~createCountTagsByItem();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCreate();

protected slots:
    virtual void languageChange();

    virtual void sPopulateLocations();


private:
    bool _captive;

};

#endif // CREATECOUNTTAGSBYITEM_H
