/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCOUNTSLIPEDITLIST_H
#define DSPCOUNTSLIPEDITLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspCountSlipEditList.h"

class dspCountSlipEditList : public XWidget, public Ui::dspCountSlipEditList
{
    Q_OBJECT

public:
    dspCountSlipEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCountSlipEditList();

    virtual void populate();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sPost();
    virtual void sPostAll();
    virtual void sCountTagList();
    virtual void sFillList();
    virtual void sHandleButtons(bool);

protected slots:
    virtual void languageChange();

private:
    int _cnttagid;

};

#endif // DSPCOUNTSLIPEDITLIST_H
