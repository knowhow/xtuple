/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COUNTSLIP_H
#define COUNTSLIP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_countSlip.h"

class countSlip : public XDialog, public Ui::countSlip
{
    Q_OBJECT

public:
    countSlip(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~countSlip();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sCatchCounttagid( int pCnttagid );
    virtual void sSave();
    virtual void sCountTagList();
    virtual void populate();
    virtual void sPopulateItemSiteInfo();
    virtual void populateTagInfo();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _cntslipid;
    int _cnttagid;
    bool _captive;

};

#endif // COUNTSLIP_H
