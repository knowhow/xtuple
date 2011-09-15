/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPBILLINGSELECTIONS_H
#define DSPBILLINGSELECTIONS_H

#include "xwidget.h"

#include "ui_dspBillingSelections.h"

class dspBillingSelections : public XWidget, public Ui::dspBillingSelections
{
    Q_OBJECT

public:
    dspBillingSelections(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspBillingSelections();

public slots:
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sFillList();
    virtual void sPostAll();
    virtual void sPost();
    virtual void sNew();
    virtual void sEdit();
    virtual void sCancel();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // DSPBILLINGSELECTIONS_H
