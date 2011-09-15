/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CORRECTPRODUCTIONPOSTING_H
#define CORRECTPRODUCTIONPOSTING_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_correctProductionPosting.h"

class correctProductionPosting : public XDialog, public Ui::correctProductionPosting
{
    Q_OBJECT

public:
    correctProductionPosting(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~correctProductionPosting();

    Q_INVOKABLE virtual bool captive() const { return _captive; }
    Q_INVOKABLE virtual bool okToPost();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void clear();

protected slots:
    virtual void languageChange();

    virtual void sCorrect();
    virtual void populate();


private:
    double _qtyReceivedCache;
    bool _captive;

};

#endif // CORRECTPRODUCTIONPOSTING_H
