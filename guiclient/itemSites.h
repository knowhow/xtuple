/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSITES_H
#define ITEMSITES_H

#include "guiclient.h"
#include "display.h"

class itemSites : public display
{
    Q_OBJECT

public:
    itemSites(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sNew();
    virtual void sView();
    virtual void sEdit();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sInventoryAvailability();
    virtual void sIssueCountTag();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
};

#endif // ITEMSITES_H
