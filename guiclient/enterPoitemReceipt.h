/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ENTERPOITEMRECEIPT_H
#define ENTERPOITEMRECEIPT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_enterPoitemReceipt.h"

class enterPoitemReceipt : public XDialog, public Ui::enterPoitemReceipt
{
    Q_OBJECT

public:
    enterPoitemReceipt(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~enterPoitemReceipt();
    
    static bool correctReceipt( int pRecvid, QWidget * pParent );

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sReceive();
    virtual void sDetermineToReceiveInv();
    virtual void sPrintItemLabel();

protected:
    bool _snooze;

protected slots:
    virtual void languageChange();

private:
    int		_mode;
    int		_orderitemid;
    QString	_ordertype;
    double	_receivable;
    int		_recvid;

};

#endif // ENTERPOITEMRECEIPT_H
