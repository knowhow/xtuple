/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOMLIST_H
#define BOMLIST_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_bomList.h"

class bomList : public XWidget, public Ui::bomList
{
    Q_OBJECT

public:
    bomList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~bomList();

public slots:
    virtual void sCopy();
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList( int pItemid, bool pLocal );
    virtual void sNew();
    virtual void sPrint();
    virtual void sSearch( const QString & pTarget );
    virtual void sView();
    virtual void sFillList();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

};

#endif // BOMLIST_H
