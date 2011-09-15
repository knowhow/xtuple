/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXCODE_H
#define TAXCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_taxCode.h"


class taxCode : public XDialog, public Ui::taxCode
{
    Q_OBJECT

public:
    taxCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~taxCode();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );

	virtual bool setParams(ParameterList &pParams); 
	virtual void initTaxCode(); 
	virtual void closeEvent( QCloseEvent * pEvent );  
    virtual void sSave();
    virtual void sCheck();
    virtual void populate();
	virtual void populateBasis();
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sFillList();
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
	virtual void sExpire();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _taxid;

};

#endif //TAXCODE_H
