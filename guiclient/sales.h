/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALES_H
#define SALES_H

#include "xwidget.h"
#include "ui_sales.h"

class sales : public XWidget, public Ui::sales
{
    Q_OBJECT

public:
    sales(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~sales();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // SALES_H
