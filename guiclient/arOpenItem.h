/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef AROPENITEM_H
#define AROPENITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_arOpenItem.h"

class arOpenItem : public XDialog, public Ui::arOpenItem
{
    Q_OBJECT

public:
    arOpenItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~arOpenItem();

    virtual void populate();

    QPushButton* _save;

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sClose();
    virtual void sPopulateCustInfo( int pCustid );
    virtual void reset();
    virtual void sCalculateCommission();
    virtual void sPopulateDueDate();
    virtual void sPrintOnPost(int temp_id);
    virtual void sTaxDetail();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _aropenid;
    int _last;
    double _cAmount;
    double _commprcnt;

};

#endif // AROPENITEM_H
