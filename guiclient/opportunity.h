/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITY_H
#define OPPORTUNITY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_opportunity.h"

class opportunity : public XDialog, public Ui::opportunity
{
    Q_OBJECT

public:
    opportunity(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~opportunity();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sCancel();
    virtual void sSave();
    virtual bool save(bool);
    virtual void sDeleteTodoItem();
    virtual void sEditTodoItem();
    virtual void sFillTodoList();
    virtual void sHandleTodoPrivs();
    virtual void sNewTodoItem();
    virtual void sPopulateTodoMenu(QMenu*);
    virtual void sViewTodoItem();
    virtual void sDeleteSale();
    virtual void sEditSale();
    virtual void sFillSalesList();
    virtual void sHandleSalesPrivs();
    virtual void sPopulateSalesMenu(QMenu*);
    virtual void sPrintSale();
    virtual void sViewSale();
    virtual void sDeleteQuote();
    virtual void sEditQuote();
    virtual void sNewQuote();
    virtual void sAttachQuote();
    virtual void sPrintQuote();
    virtual void sConvertQuote();
    virtual void sViewQuote();
    virtual void sDeleteSalesOrder();
    virtual void sEditSalesOrder();
    virtual void sNewSalesOrder();
    virtual void sAttachSalesOrder();
    virtual void sPrintSalesOrder();
    virtual void sViewSalesOrder();
    virtual void sFillCharList();
    virtual void sNewCharacteristic();
    virtual void sEditCharacteristic();
    virtual void sDeleteCharacteristic();
	virtual void sHandleCrmacct(int);

protected slots:
    virtual void languageChange();

private:
    int		_opheadid;
	int		_custid;
	int		_prospectid;
    int		_mode;
    bool	_saved;
};

#endif // OPPORTUNITY_H
