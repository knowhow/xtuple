/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FAILEDPOSTLIST_H
#define FAILEDPOSTLIST_H

#include "guiclient.h"
#include "xdialog.h"
#include <QHeaderView>

#include "ui_failedPostList.h"

class failedPostList : public XDialog, public Ui::failedPostList
{
    Q_OBJECT

public:
    failedPostList(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~failedPostList();

public slots:
    virtual void sSetList(QList<XTreeWidgetItem*>, QTreeWidgetItem*, QHeaderView*);
    virtual void setLabel(const QString&);

protected slots:
    virtual void languageChange();

};

#endif // FAILEDPOSTLIST_H
