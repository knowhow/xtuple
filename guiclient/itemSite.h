/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSITE_H
#define ITEMSITE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_itemSite.h"

class itemSite : public XDialog, public Ui::itemSite
{
    Q_OBJECT

public:
    itemSite(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~itemSite();
    static int		createItemSite(QWidget*, int, int, bool);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool sSave();
    virtual void sCheckItemsite();
    virtual void sHandlePlanningType();
    virtual void sHandleJobCost();
    virtual void sHandlePOSupplied( bool pSupplied );
    virtual void sHandleCreatePO( bool pCreate );
    virtual void sHandleCreateSOPR( bool pCreate );
    virtual void sHandleCreateWOPR( bool pCreate );
    virtual void sHandleWOSupplied( bool pSupplied );
    virtual void sHandleControlMethod();
    virtual void sCacheItemType( const QString & pItemType );
    virtual void sCacheItemType( char pItemType );
    virtual void populateLocations();
    virtual void populate();
    virtual void clear();
    virtual void sFillRestricted();
    virtual void sToggleRestricted();
    virtual void sDefaultLocChanged();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _itemsiteid;
    bool _captive;
    bool _updates;
    double _qohCache;
    char _itemType;
    bool _wasLotSerial;
    bool _wasLocationControl;

};

#endif // ITEMSITE_H
