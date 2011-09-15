/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MISCCHECK_H
#define MISCCHECK_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_miscCheck.h"

class miscCheck : public XWidget, public Ui::miscCheck
{
    Q_OBJECT

public:
    miscCheck(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~miscCheck();

    virtual void populate();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sHandleButtons();
    virtual void sPopulateBankInfo( int pBankaccntid );
    virtual void sSave();
	virtual void sCustomerSelected();
	virtual void sCreditMemoSelected();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    int _checkid;
    int _mode;
	int _raheadid;
	int _aropenid;
	double _aropenamt;

};

#endif // MISCCHECK_H
