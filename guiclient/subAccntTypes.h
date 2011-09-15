/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SUBACCNTTYPES_H
#define SUBACCNTTYPES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_subAccntTypes.h"

class subAccntTypes : public XWidget, public Ui::subAccntTypes
{
    Q_OBJECT

public:
    subAccntTypes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~subAccntTypes();

protected slots:
    virtual void languageChange();

    virtual void sNew();
    virtual void sPrint();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();

};

#endif // SUBACCNTTYPES_H
