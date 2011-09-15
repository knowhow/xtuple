/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef quotelist_h
#define quotelist_h

#include <qdialog.h>

#include "widgets.h"

class WarehouseGroup;
class QPushButton;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT quoteList : public QDialog
{
    Q_OBJECT

public:
    quoteList( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );

    WarehouseGroup *_warehouse;
    QPushButton    *_close;
    QPushButton    *_select;
    XTreeWidget      *_qu;

public slots:
    virtual void set( ParameterList & pParams );
    virtual void sSelect();
    virtual void sFillList();


private:
    int _quheadid;
    int _type;
    int _custid;
};

#endif

