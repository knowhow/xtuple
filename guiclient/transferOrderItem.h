/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSFERORDERITEM_H
#define TRANSFERORDERITEM_H

#include "guiclient.h"
#include <QStandardItemModel>
#include "xdialog.h"
#include <parameter.h>
#include "ui_transferOrderItem.h"

class transferOrderItem : public XDialog, public Ui::transferOrderItem
{
    Q_OBJECT

public:
    transferOrderItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~transferOrderItem();

    virtual void prepare();
    virtual void clear();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sPopulateItemInfo( int pItemid );
    virtual void sDetermineAvailability();
    virtual void populate();
    virtual void sNext();
    virtual void sPrev();
    virtual void sChanged();
    virtual void sCancel();
    virtual void sCalculateTax();
    virtual void sTaxDetail();
    virtual void sHandleButton();

protected slots:
    virtual void languageChange();
    virtual void reject();


private:
    int			_availabilityLastItemid;
    QDate		_availabilityLastSchedDate;
    bool		_availabilityLastShow;
    bool		_availabilityLastShowIndent;
    int			_availabilityLastWarehousid;
    double	_availabilityQtyOrdered;
    bool		_canceling;
    bool    _captive;
    int			_dstwhsid;
    bool		_error;
    QStandardItemModel	*_itemchar;
    int			_itemsiteid;
    int			_mode;
    bool		_modified;
    double	_originalQtyOrd;
    int			_taxzoneid;
    int			_toheadid;
    int			_toitemid;
    int			_transwhsid;
    bool    _saved;
};

#endif // TRANSFERORDERITEM_H
