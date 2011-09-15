/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DSPBOMBASE_H__
#define __DSPBOMBASE_H__

#include "display.h"
#include "guiclient.h"
#include <parameter.h>

#include "ui_dspBOMBase.h"

class dspBOMBase : public display, public Ui::dspBOMBase
{
    Q_OBJECT

public:
    dspBOMBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &params);

public slots:
    virtual void sEdit();
    virtual void sView();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual enum SetResponse set(const ParameterList & pParams);

protected slots:
    virtual void languageChange();

};

#endif //__DSPBOMBASE_H__
