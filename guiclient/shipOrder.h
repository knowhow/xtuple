/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIPORDER_H
#define SHIPORDER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_shipOrder.h"

class shipOrder : public XDialog, public Ui::shipOrder
{
    Q_OBJECT

public:
    shipOrder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~shipOrder();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sShip();
    virtual void sFillList();
    virtual void sFillFreight();
    virtual void sFillTracknum();
    virtual void sHandleOrder();
    virtual void sHandleSo();
    virtual void sHandleTo();
    virtual void sHandleButtons();
	virtual void calcFreight();

protected slots:
    virtual void languageChange();

    virtual void sSelectToggled( bool yes );
    virtual void sCreateToggled( bool yes );


private:
    bool	_captive;
    bool	_reject;	// for handling errors during set()

};

#endif // SHIPORDER_H
