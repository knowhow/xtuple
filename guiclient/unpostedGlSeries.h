/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDGLSERIES_H
#define UNPOSTEDGLSERIES_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_unpostedGlSeries.h"

class unpostedGlSeries : public XWidget, public Ui::unpostedGlSeries
{
    Q_OBJECT

public:
    unpostedGlSeries(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unpostedGlSeries();

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPopulateMenu(QMenu * pMenu);
    virtual void sPost();
    virtual void sPrint();
    virtual void sView();

protected slots:
    virtual void languageChange();

protected:
    void removeDupAltIds(QList<XTreeWidgetItem*>&);

};

#endif // UNPOSTEDGLSERIES_H
