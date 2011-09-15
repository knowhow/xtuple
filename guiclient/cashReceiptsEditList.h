/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CASHRECEIPTSEDITLIST_H
#define CASHRECEIPTSEDITLIST_H

#include "xwidget.h"

#include "ui_cashReceiptsEditList.h"

class cashReceiptsEditList : public XWidget, public Ui::cashReceiptsEditList
{
    Q_OBJECT

public:
    cashReceiptsEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~cashReceiptsEditList();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPost();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // CASHRECEIPTSEDITLIST_H
