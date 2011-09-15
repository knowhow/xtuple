/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDGLTRANSACTIONS_H
#define UNPOSTEDGLTRANSACTIONS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_unpostedGLTransactions.h"

class unpostedGLTransactions : public XDialog, public Ui::unpostedGLTransactions
{
    Q_OBJECT

public:
    unpostedGLTransactions(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~unpostedGLTransactions();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*);
    virtual void sPrint();
    virtual void sFillList();
    virtual void sViewTrans();
    virtual void sViewDocument();

protected slots:
    virtual void languageChange();

private:
    int		_periodid;

};

#endif // UNPOSTEDGLTRANSACTIONS_H
