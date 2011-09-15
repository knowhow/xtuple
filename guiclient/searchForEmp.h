/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SEARCHFOREMP_H
#define SEARCHFOREMP_H

#include <parameter.h>

#include "guiclient.h"
#include "xwidget.h"
#include "ui_searchForEmp.h"

class searchForEmp : public XWidget, public Ui::searchForEmp
{
    Q_OBJECT

public:
    searchForEmp(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~searchForEmp();

    static bool userHasPriv(const int = cView);
    virtual void setVisible(bool);

public slots:
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *pSelected );
    virtual void sEdit();
    virtual void sEditMgr();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPrint();
    virtual void sView();
    virtual void sViewMgr();
    virtual bool setParams(ParameterList &);


protected slots:
    virtual void languageChange();

};

#endif // SEARCHFOREMP_H
