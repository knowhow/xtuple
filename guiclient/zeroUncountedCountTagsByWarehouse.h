/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ZEROUNCOUNTEDCOUNTTAGSBYWAREHOUSE_H
#define ZEROUNCOUNTEDCOUNTTAGSBYWAREHOUSE_H

#include "xdialog.h"
#include "ui_zeroUncountedCountTagsByWarehouse.h"

class zeroUncountedCountTagsByWarehouse : public XDialog, public Ui::zeroUncountedCountTagsByWarehouse
{
    Q_OBJECT

public:
    zeroUncountedCountTagsByWarehouse(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~zeroUncountedCountTagsByWarehouse();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void sZero();


};

#endif // ZEROUNCOUNTEDCOUNTTAGSBYWAREHOUSE_H
