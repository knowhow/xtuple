/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "workcentercluster.h"

WorkCenterCluster::WorkCenterCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new WorkCenterLineEdit(this, pName));
}

WorkCenterLineEdit::WorkCenterLineEdit(QWidget *pParent, const char *pName)
  : VirtualClusterLineEdit(pParent, "xtmfg.wrkcnt", "wrkcnt_id", "wrkcnt_code", "wrkcnt_descrip", 0, 0, pName)
{
  setTitles(tr("Work Center"), tr("Work Centers"));
}

void WorkCenterLineEdit::setWorkCenter(const QString & pWorkCenter)
{
  setNumber(pWorkCenter);
}
