/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTAXHISTORY_H
#define DSPTAXHISTORY_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_dspTaxHistory.h"

class dspTaxHistory : public XWidget, public Ui::dspTaxHistory
{
    Q_OBJECT

public:
    dspTaxHistory(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTaxHistory();
    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sFillList();
    virtual void sHandleFilter();
    virtual void sHandleType();

protected slots:
    virtual void languageChange();

};

#endif // DSPTAXHISTORY_H
