/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef salesorderlist_h
#define salesorderlist_h

#include <QDialog>

#include "widgets.h"

class WarehouseGroup;
class QPushButton;
class XTreeWidget;
class ParameterList;

//  Possible Customer Order Status
#define cSoOpen               0x01
#define cSoClosed             0x02
#define cSoAtShipping         0x04
#define cSoReleased           0x08
#define cSoCustomer           0x16


class XTUPLEWIDGETS_EXPORT salesOrderList : public QDialog
{
    Q_OBJECT

public:
    salesOrderList( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );

    WarehouseGroup *_warehouse;
    QPushButton    *_close;
    QPushButton    *_select;
    XTreeWidget      *_so;

public slots:
    virtual void set(const ParameterList & pParams);
    virtual void sSelect();
    virtual void sFillList();


private:
    int _soheadid;
    int _type;
    int _custid;
};

#endif

