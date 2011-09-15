/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BANKADJUSTMENT_H
#define BANKADJUSTMENT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_bankAdjustment.h"

class bankAdjustment : public XWidget, public Ui::bankAdjustment
{
    Q_OBJECT

public:
    bankAdjustment(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~bankAdjustment();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sBankAccount( int accountId );

protected:
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _bankadjid;

};

#endif // BANKADJUSTMENT_H
