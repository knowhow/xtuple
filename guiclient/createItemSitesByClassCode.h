/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREATEITEMSITESBYCLASSCODE_H
#define CREATEITEMSITESBYCLASSCODE_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_createItemSitesByClassCode.h"

class createItemSitesByClassCode : public XDialog, public Ui::createItemSitesByClassCode
{
    Q_OBJECT

public:
    createItemSitesByClassCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~createItemSitesByClassCode();

public slots:
    virtual void sSave();
    virtual void sHandlePOSupply( bool pSupplied );
    virtual void sHandleWOSupply( bool pSupplied );
    virtual void sHandleControlMethod();
    virtual void sHandlePlanningType();
    virtual void populateLocations();
    virtual void clear();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _locationGroupInt;
};

#endif // CREATEITEMSITESBYCLASSCODE_H
