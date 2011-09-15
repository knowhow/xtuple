/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef purchaseorderlist_h
#define purchaseorderlist_h

#include <QDialog>

#include "widgets.h"
#include "vendorcluster.h"

class XTreeWidget;
class QPushButton;
class ParameterList;

class XTUPLEWIDGETS_EXPORT purchaseOrderList : public QDialog
{
    Q_OBJECT

public:
    purchaseOrderList( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );

    XTreeWidget* _pohead;
    QPushButton* _close;
    QPushButton* _select;

public slots:
    virtual void set( ParameterList & pParams );
    virtual void sFillList();
    virtual void sSelect();
    virtual void sClose();


private:
    int _poheadid;
    int _type;
    VendorCluster *_vend;

};

#endif
