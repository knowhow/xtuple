/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PACKAGES_H
#define PACKAGES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_packages.h"

class packages : public XWidget, public Ui::packages
{
    Q_OBJECT

public:
    packages(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~packages();

public slots:
    virtual void sDelete();
    virtual void sDisable();
    virtual void sEdit();
    virtual void sEnable();
    virtual void sFillList();
    virtual void sHandleAutoUpdate(const bool);
    virtual void sHandleButtons(const bool);
    virtual void sLoad();
    virtual void sNew();
    virtual void sPopulateMenu( QMenu * );
    virtual void sPrint();
    virtual void sView();

protected slots:
    virtual void languageChange();

};

#endif // PACKAGES_H
