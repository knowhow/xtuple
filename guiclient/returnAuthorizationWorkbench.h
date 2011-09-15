/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RETURNAUTHORIZATIONWORKBENCH_H
#define RETURNAUTHORIZATIONWORKBENCH_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_returnAuthorizationWorkbench.h"

class returnAuthorizationWorkbench : public XWidget, public Ui::returnAuthorizationWorkbench
{
    Q_OBJECT

public:
    returnAuthorizationWorkbench(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~returnAuthorizationWorkbench();
  
    virtual bool checkSitePrivs(int ordid);

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sPrintDue();
    virtual void sEditDue();
    virtual void sViewDue();
    virtual void sProcess();
    virtual void sHandleButton();
    virtual void sFillLists();
    virtual void sFillListReview();
    virtual void sFillListDue();
    virtual void sPopulateReviewMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sPopulateDueMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );

protected slots:
    virtual void languageChange();
    virtual void setParams(ParameterList&);

};

#endif // RETURNAUTHORIZATIONWORKBENCH_H
