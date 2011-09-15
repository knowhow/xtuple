/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ISSUELINETOSHIPPING_H
#define ISSUELINETOSHIPPING_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_issueLineToShipping.h"

class issueLineToShipping : public XDialog, public Ui::issueLineToShipping
{
    Q_OBJECT

public:
    issueLineToShipping(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~issueLineToShipping();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sIssue();

protected:
    bool _requireInventory;
    bool _snooze;

protected slots:
    virtual void languageChange();
    virtual void populate();

private:
    int		_itemid;
    QString	_ordertype;
    QDateTime   _transTS;

};

#endif // ISSUELINETOSHIPPING_H
