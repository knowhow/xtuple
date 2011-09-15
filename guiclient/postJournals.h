/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its postJournals.
 */

#ifndef POSTJOURNALS_H
#define POSTJOURNALS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_postJournals.h"

class postJournals : public XWidget, public Ui::postJournals
{
    Q_OBJECT

public:
    postJournals(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~postJournals();

public slots:
    void sPost();
    void sFillList();
    void sHandlePreview();
    void sHandleSelection();
    void sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *);
    void sViewTransactions();
    void sPrint(QList<int>);

protected slots:
    void languageChange();
};

#endif // POSTJOURNALS_H
