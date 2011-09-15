/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPQUOTESBYITEM_H
#define DSPQUOTESBYITEM_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspQuotesByItem.h"

class dspQuotesByItem : public display, public Ui::dspQuotesByItem
{
    Q_OBJECT

public:
    dspQuotesByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool checkSitePrivs(int orderid);
    virtual bool setParams(ParameterList&);

public slots:
    virtual SetResponse set(const ParameterList & pParams);
    virtual void sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int);
    virtual void sEditOrder();
    virtual void sViewOrder();

protected slots:
    virtual void languageChange();

};

#endif // DSPQUOTESBYITEM_H
