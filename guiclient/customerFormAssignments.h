/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CUSTOMERFORMASSIGNMENTS_H
#define CUSTOMERFORMASSIGNMENTS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_customerFormAssignments.h"

class customerFormAssignments : public XWidget, public Ui::customerFormAssignments
{
    Q_OBJECT

public:
    customerFormAssignments(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~customerFormAssignments();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // CUSTOMERFORMASSIGNMENTS_H
