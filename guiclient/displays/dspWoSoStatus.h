/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOSOSTATUS_H
#define DSPWOSOSTATUS_H

#include "display.h"

#include "ui_dspWoSoStatus.h"

class dspWoSoStatus : public display, public Ui::dspWoSoStatus
{
    Q_OBJECT

public:
    dspWoSoStatus(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList & );

public slots:
    virtual void sCloseWo();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected, int);
    virtual void sViewWomatlreq();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOSOSTATUS_H
