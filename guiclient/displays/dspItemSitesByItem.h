/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPITEMSITESBYITEM_H
#define DSPITEMSITESBYITEM_H

#include "display.h"

#include "ui_dspItemSitesByItem.h"

class dspItemSitesByItem : public display, public Ui::dspItemSitesByItem
{
    Q_OBJECT

public:
    dspItemSitesByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPopulateMenu(QMenu * menu, QTreeWidgetItem * pSelected, int);
    virtual void sViewItemsite();
    virtual void sEditItemsite();
    virtual void sViewInventoryAvailability();
    virtual void sViewLocationLotSerialDetail();
    virtual void sIssueCountTag();

protected slots:
    virtual void languageChange();

};

#endif // DSPITEMSITESBYITEM_H
