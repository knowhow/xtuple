/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByItem.h"
#include "inputManager.h"

dspUsageStatisticsByItem::dspUsageStatisticsByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : dspUsageStatisticsBase(parent, "dspUsageStatisticsByItem", fl)
{
  setWindowTitle(tr("Item Usage Statistics by Item"));
  setReportName("UsageStatisticsByItem");
  _parameter->hide();

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));
}

