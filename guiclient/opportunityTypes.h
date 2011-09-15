/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITYTYPES_H
#define OPPORTUNITYTYPES_H

#include "xwidget.h"
#include "ui_opportunityTypes.h"

class opportunityTypes : public XWidget, public Ui::opportunityTypes
{
    Q_OBJECT

public:
    opportunityTypes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~opportunityTypes();

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // OPPORTUNITYTYPES_H
