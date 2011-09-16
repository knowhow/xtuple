/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSITES_H
#define ITEMSITES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_itemSites.h"

class itemSites : public XWidget, public Ui::itemSites
{
    Q_OBJECT

public:
    itemSites(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemSites();

public slots:
    virtual void sNew();
    virtual void sCopy();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();
    virtual void sSearch( const QString & pTarget );

protected:
    virtual void keyPressEvent( QKeyEvent * e );

protected slots:
    virtual void languageChange();

};

#endif // ITEMSITES_H
