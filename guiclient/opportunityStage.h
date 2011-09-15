/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITYSTAGE_H
#define OPPORTUNITYSTAGE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_opportunityStage.h"

class opportunityStage : public XDialog, public Ui::opportunityStage
{
    Q_OBJECT

public:
    opportunityStage(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~opportunityStage();

    virtual void populate();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _opstageid;

};

#endif // OPPORTUNITYSTAGE_H
