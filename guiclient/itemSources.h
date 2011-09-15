/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSOURCES_H
#define ITEMSOURCES_H

#include "display.h"

class itemSources : public display
{
    Q_OBJECT

public:
    itemSources(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sBuyCard();
};

#endif // ITEMSOURCES_H
