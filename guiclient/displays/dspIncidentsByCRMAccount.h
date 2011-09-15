/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINCIDENTSBYCRMACCOUNT_H
#define DSPINCIDENTSBYCRMACCOUNT_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspIncidentsByCRMAccount.h"

class dspIncidentsByCRMAccount : public display, public Ui::dspIncidentsByCRMAccount
{
    Q_OBJECT

public:
    dspIncidentsByCRMAccount(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sEditCRMAccount();
    virtual void sEditIncident();
    virtual void sEditTodoItem();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem*, int);
    virtual void sViewCRMAccount();
    virtual void sViewIncident();
    virtual void sViewTodoItem();

protected slots:
    virtual void languageChange();
};
#endif // DSPINCIDENTSBYCRMACCOUNT_H
