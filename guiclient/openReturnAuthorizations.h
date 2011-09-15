/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPENRETURNAUTHORIZATIONS_H
#define OPENRETURNAUTHORIZATIONS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_openReturnAuthorizations.h"

class openReturnAuthorizations : public XWidget, public Ui::openReturnAuthorizations
{
    Q_OBJECT

public:
    openReturnAuthorizations(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~openReturnAuthorizations();
    
    virtual void setParams(ParameterList &);
    virtual bool checkSitePrivs(int ordid);

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();
//    virtual void sDeliver();
    virtual void sPrintForms(); 

protected slots:
    virtual void languageChange();

};

#endif // OPENRETURNAUTHORIZATIONS_H
