/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXDETAIL_H
#define TAXDETAIL_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_taxDetail.h"

class taxDetail : public XDialog, public Ui::taxDetail
{
    Q_OBJECT

public:
    taxDetail(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~taxDetail();

	virtual int    taxtype()	const;

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void clear();
    virtual void sCancel();
    virtual void sCalculateTax();
    virtual void sPopulate();
    virtual void sNew();
    virtual void sDelete();

protected:
    double	_aCache;
    double	_bCache;
    double	_cCache;
    double	_aPctCache;
    double	_bPctCache;
    double	_cPctCache;
    bool	_blankDetailDescriptions;
    bool	_readonly;
    double	_taxCodeInitialized;
    int         _taxzoneId;
    int		_orderid;
    int         _sense;
    QString	_ordertype;
    QString     _displayType;
    bool        _adjustment;
   
protected slots:
    virtual void languageChange();
};

#endif // TAXDETAIL_H
