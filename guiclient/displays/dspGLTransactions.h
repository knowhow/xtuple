/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPGLTRANSACTIONS_H
#define DSPGLTRANSACTIONS_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspGLTransactions.h"

class dspGLTransactions : public display, public Ui::dspGLTransactions
{
    Q_OBJECT

public:
    dspGLTransactions(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &params);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*, int);
    virtual void sPrint();
    virtual void sFillList(ParameterList, bool = false);
    virtual void sViewTrans();
    virtual void sViewSeries();
    virtual void sViewDocument();
    virtual void sViewJournal();

protected slots:
    virtual void languageChange();
    virtual bool forwardUpdate();

private slots:
    void handleTotalCheckbox();

private:
    QStringList _sources;

};

#endif // DSPGLTRANSACTIONS_H
