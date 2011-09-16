/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSOURCES_H
#define ITEMSOURCES_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_itemSources.h"

class itemSources : public XWidget, public Ui::itemSources
{
    Q_OBJECT

public:
    itemSources(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemSources();

protected slots:
    virtual void languageChange();

    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sFillList();
    virtual void sSearch( const QString & pTarget );


};

#endif // ITEMSOURCES_H
