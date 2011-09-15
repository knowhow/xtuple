/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXAUTHORITIES_H
#define TAXAUTHORITIES_H

#include "display.h"

class taxAuthorities : public display
{
    Q_OBJECT

public:
    taxAuthorities(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
};

#endif // TAXAUTHORITIES_H
