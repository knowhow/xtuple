/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_department.h"

class department : public XDialog, public Ui::department
{
    Q_OBJECT

public:
    department(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~department();

public slots:
    virtual SetResponse set( const ParameterList & );
    virtual void sSave();
    virtual void sClose();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _deptid;
};

#endif // DEPARTMENT_H
