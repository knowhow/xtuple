/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __WORKCENTERCLUSTER_H__
#define __WORKCENTERCLUSTER_H__

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT WorkCenterLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public:
    WorkCenterLineEdit(QWidget*, const char* = 0);

    void setWorkCenter(const QString &);
};

class XTUPLEWIDGETS_EXPORT WorkCenterCluster : public VirtualCluster
{
  Q_OBJECT

  public:
    WorkCenterCluster(QWidget*, const char* = 0);
    inline void setWorkCenter(const QString & pWorkCenter) { setNumber(pWorkCenter); }
    inline const QString workcenter() const { return _number->text(); }

};

#endif
