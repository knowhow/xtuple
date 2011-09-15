/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DISTRIBUTEINVENTORY_H
#define DISTRIBUTEINVENTORY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_distributeInventory.h"

class distributeInventory : public XDialog, public Ui::distributeInventory
{
    Q_OBJECT

public:
    distributeInventory(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~distributeInventory();

    static int SeriesAdjust( int pItemlocSeries, QWidget * pParent, const QString & = QString::null, const QDate & = QDate(), const QDate & = QDate() );
    virtual enum SetResponse set( const ParameterList & pParams );

public slots:
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void populate();
    virtual void sBcChanged(const QString);
    virtual void sBcDistribute();
    virtual void sCatchLotSerialNumber(const QString);
    virtual bool sDefault();
    virtual void sDefaultAndPost();
    virtual void sFillList();
    virtual void sPost();
    virtual void sSelectLocation();
    virtual void sPopulateDefaultSelector();
    virtual void sChangeDefaultLocation();

protected slots:
    virtual void languageChange();

private:
    QString	_controlMethod;
    int		_itemlocdistid;
    int		_mode;
    int   _itemsite_id;

};

#endif // DISTRIBUTEINVENTORY_H
