/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ALLOCATEARCREDITMEMO_H
#define ALLOCATEARCREDITMEMO_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_allocateARCreditMemo.h"

class allocateARCreditMemo : public XDialog, public Ui::allocateARCreditMemo
{
    Q_OBJECT

public:
    allocateARCreditMemo(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~allocateARCreditMemo();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCancel();
    virtual void sPopulate();
    virtual void sAllocate();
    virtual void sHandleButton();

protected:
    bool	_readonly;
    bool        _allocateMode;
    int         _custid;
    int		_coheadid;
    int         _invcheadid;
    QString	_doctype;
   
protected slots:
    virtual void languageChange();
};

#endif // ALLOCATEARCREDITMEMO_H
