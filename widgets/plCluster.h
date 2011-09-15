/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef plCluster_h
#define plCluster_h

#include "virtualCluster.h"

#include "widgets.h"

class QLabel;
class QPushButton;

class PlanOrdCluster;

#define WAREHOUSE       5
#define ITEM_NUMBER     6
#define UOM             7
#define ITEM_DESCRIP1   8
#define ITEM_DESCRIP2   9

class XTUPLEWIDGETS_EXPORT PlanOrdLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public:
    PlanOrdLineEdit(QWidget*, const char* = 0);

};

class XTUPLEWIDGETS_EXPORT PlanOrdCluster : public VirtualCluster
{
  Q_OBJECT

  public:
    PlanOrdCluster(QWidget*, const char* = 0);

  public slots:
    void sRefresh();

  private:
    QLabel           *_warehouse;
    QLabel           *_itemNumber;
    QLabel           *_uom;
    QLabel           *_descrip1;
    QLabel           *_descrip2;

};

#endif

