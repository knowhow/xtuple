/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SCRAPTRANS_H
#define SCRAPTRANS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_scrapTrans.h"

class scrapTrans : public XWidget, public Ui::scrapTrans
{
    Q_OBJECT

public:
    scrapTrans(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~scrapTrans();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPost();
    virtual void sPopulateQOH( int pWarehousid );
    virtual void sPopulateQty();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    bool _captive;
    double _cachedQOH;

};

#endif // SCRAPTRANS_H
