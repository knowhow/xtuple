/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTPRODUCTION_H
#define POSTPRODUCTION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_postProduction.h"

class postProduction : public XDialog, public Ui::postProduction
{
    Q_OBJECT

public:
    postProduction(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postProduction();

    Q_INVOKABLE bool    captive() { return _captive; }
    Q_INVOKABLE QString handleSeriesAdjustAfterPost(int itemlocSeries);
    Q_INVOKABLE QString handleTransferAfterPost();
    Q_INVOKABLE bool    okToPost();
    Q_INVOKABLE QString updateWoAfterPost();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void clear();
    virtual void sHandleWoid( int pWoid );
    virtual void sReadWorkOrder( int pWoid );
    virtual void sPost();
    virtual void sScrap();
    virtual void sCatchWoid( int pWoid );

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // POSTPRODUCTION_H
