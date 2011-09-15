/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTAROPENITEM_H
#define PRINTAROPENITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_printArOpenItem.h"

class printArOpenItem : public XDialog, public Ui::printArOpenItem
{
    Q_OBJECT

public:
    printArOpenItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printArOpenItem();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );

signals:
            void finishedPrinting(int);

protected slots:
    virtual void languageChange();

    virtual void sPrint();


private:
    bool _captive;

};

#endif // PRINTAROPENITEM_H
