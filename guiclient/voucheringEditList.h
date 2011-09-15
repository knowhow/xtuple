/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VOUCHERINGEDITLIST_H
#define VOUCHERINGEDITLIST_H

#include "xwidget.h"
#include "ui_voucheringEditList.h"

class voucheringEditList : public XWidget, public Ui::voucheringEditList
{
    Q_OBJECT

public:
    voucheringEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~voucheringEditList();

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // VOUCHERINGEDITLIST_H
