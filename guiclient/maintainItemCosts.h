/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MAINTAINITEMCOSTS_H
#define MAINTAINITEMCOSTS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_maintainItemCosts.h"

class maintainItemCosts : public XWidget, public Ui::maintainItemCosts
{
    Q_OBJECT

public:
    maintainItemCosts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~maintainItemCosts();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sViewDetail();
    virtual void sUpdateDetail();
    virtual void sUpdateDirectLabor();
    virtual void sUpdateOverhead();
    virtual void sUpdateMachineOverhead();
    virtual void sPost();
    virtual void sDelete();
    virtual void sEnterActualCost();
    virtual void sCreateUserCost();
    virtual void sNew();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sSelectionChanged();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // MAINTAINITEMCOSTS_H
