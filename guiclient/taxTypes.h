/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXTYPES_H
#define TAXTYPES_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_taxTypes.h"

class taxTypes : public XWidget, public Ui::taxTypes
{
    Q_OBJECT

public:
    taxTypes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~taxTypes();

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList(int pId);
    virtual void sPrint();

protected slots:
    virtual void languageChange();
};

#endif // TAXTYPES_H
