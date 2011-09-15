/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EXTERNALCCTRANSACTION_H
#define EXTERNALCCTRANSACTION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_externalCCTransaction.h"

class externalCCTransaction : public XDialog, public Ui::externalCCTransaction
{
    Q_OBJECT

public:
    externalCCTransaction(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~externalCCTransaction();

public slots:
    virtual enum SetResponse set(const ParameterList &pParams);
    virtual void getResults(ParameterList &);

protected slots:
    virtual void languageChange();

private:

};

#endif // EXTERNALCCTRANSACTION_H
