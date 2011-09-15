/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DSPCOSTEDBOMBASE_H__
#define __DSPCOSTEDBOMBASE_H__

#include "display.h"

#include "ui_dspCostedBOMBase.h"

class dspCostedBOMBase : public display, public Ui::dspCostedBOMBase
{
    Q_OBJECT

public:
    dspCostedBOMBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &params);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected, int);
    virtual void sMaintainItemCosts();
    virtual void sViewItemCosting();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool );

protected slots:
    virtual void languageChange();

};

#endif // __DSPCOSTEDBOMBASE_H__
