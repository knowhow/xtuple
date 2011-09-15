/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSFERORDERLIST_H
#define TRANSFERORDERLIST_H

#include <QDialog>

#include "widgets.h"

class WarehouseGroup;
class QPushButton;
class XTreeWidget;
class ParameterList;

//  Transfer Order Statuses of interest
#define cToOpen               0x01
#define cToClosed             0x02
#define cToAtShipping         0x04

class XTUPLEWIDGETS_EXPORT transferOrderList : public QDialog
{
    Q_OBJECT

public:
    transferOrderList( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );

    WarehouseGroup   *_dstwhs;
    WarehouseGroup   *_srcwhs;
    QPushButton      *_close;
    QPushButton      *_select;
    XTreeWidget      *_to;

public slots:
    virtual void set(const ParameterList & pParams );
    virtual void sSelect();
    virtual void sFillList();


private:
    int _toheadid;
    int _type;
};

#endif
