/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VOUCHERITEM_H
#define VOUCHERITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_voucherItem.h"

class voucherItem : public XDialog, public Ui::voucherItem
{
    Q_OBJECT

public:
    voucherItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~voucherItem();
	
public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sToggleReceiving( QTreeWidgetItem * pItem );
    virtual void sFillList();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem* );
    virtual void sCorrectReceiving();
    virtual void sSplitReceipt();
    virtual void reject();
	virtual void sCalculateTax();
	virtual void sTaxDetail();
    
protected:
    virtual void closeEvent(QCloseEvent * event);

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _voheadid;
    int _voitemid;
    int _poitemid;
	int _taxzoneid;
    bool _inTransaction;
	bool _saved;
	double _distamount;
    QString _rejectedMsg;
};

#endif // VOUCHERITEM_H
