/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BANKADJUSTMENTEDITLIST_H
#define BANKADJUSTMENTEDITLIST_H

#include "xwidget.h"

#include "ui_bankAdjustmentEditList.h"

class bankAdjustmentEditList : public XWidget, public Ui::bankAdjustmentEditList
{
    Q_OBJECT

public:
    bankAdjustmentEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~bankAdjustmentEditList();

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sPost();
};

#endif // BANKADJUSTMENTEDITLIST_H
