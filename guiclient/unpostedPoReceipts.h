/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDPORECEIPTS_H
#define UNPOSTEDPORECEIPTS_H

#include "guiclient.h"
#include "xwidget.h"

#include <parameter.h>

#include "ui_unpostedPoReceipts.h"

class unpostedPoReceipts : public XWidget, public Ui::unpostedPoReceipts
{
    Q_OBJECT

public:
    unpostedPoReceipts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unpostedPoReceipts();

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *);
    virtual void sPost();
    virtual void sPrint();
    virtual void sViewOrderItem();
    virtual void setParams(ParameterList &);

protected slots:
    virtual void languageChange();

private:
    QList<int> _soheadid;

};

#endif // UNPOSTEDPORECEIPTS_H
