/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSINGLELEVELWHEREUSED_H
#define DSPSINGLELEVELWHEREUSED_H

#include "display.h"

#include "ui_dspSingleLevelWhereUsed.h"

class dspSingleLevelWhereUsed : public display, public Ui::dspSingleLevelWhereUsed
{
    Q_OBJECT

public:
    dspSingleLevelWhereUsed(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu * menu, QTreeWidgetItem *, int);
    virtual void sEditBOM();
    virtual void sEditItem();
    virtual void sViewInventoryHistory();

protected slots:
    virtual void languageChange();

};

#endif // DSPSINGLELEVELWHEREUSED_H
