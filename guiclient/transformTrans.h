/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSFORMTRANS_H
#define TRANSFORMTRANS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_transformTrans.h"

class transformTrans : public XWidget, public Ui::transformTrans
{
    Q_OBJECT

public:
    transformTrans(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~transformTrans();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sPopulateQOH();
    virtual void sPopulateTarget(int pItemid);
    virtual void sPost();
    virtual void sRecalculateAfter();

protected slots:
    virtual void languageChange();

private:
    int  _mode;
    bool _captive;
    bool _controlled;
    bool _targetIsValid;

};

#endif // TRANSFORMTRANS_H
