/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDORTYPES_H
#define VENDORTYPES_H

#include "xwidget.h"
#include "ui_vendorTypes.h"

class vendorTypes : public XWidget, public Ui::vendorTypes
{
    Q_OBJECT

public:
    vendorTypes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~vendorTypes();

public slots:
    virtual void sFillList();
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sPopulateMenu( QMenu * );
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // VENDORTYPES_H
