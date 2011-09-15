/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MAINTAINBUDGET_H
#define MAINTAINBUDGET_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_maintainBudget.h"

class QTableWidgetItem;

class maintainBudget : public XWidget, public Ui::maintainBudget
{
    Q_OBJECT

public:
    maintainBudget(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~maintainBudget();

    Q_INVOKABLE int projectId() { return _prjid; }
    Q_INVOKABLE void setProjectId(int p) { _prjid = p; }

public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sAccountsAdd();
    virtual void sAccountsRemove();
    virtual void sGenerateTable();
    virtual void sPeriodsAll();
    virtual void sPeriodsInvert();
    virtual void sPrint();
    virtual void sSave();
    virtual void sValueChanged(QTableWidgetItem *item);

protected:
    virtual void closeEvent( QCloseEvent * e );
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _budgheadid;
    bool _dirty;
    int _mode;
    int _prjid;
    QList<int> _accountsRef;
    QList<int> _periodsRef;

};

#endif // MAINTAINBUDGET_H
