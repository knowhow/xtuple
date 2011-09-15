/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREATECYCLECOUNTTAGS_H
#define CREATECYCLECOUNTTAGS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_createCycleCountTags.h"

class createCycleCountTags : public XDialog, public Ui::createCycleCountTags
{
    Q_OBJECT

public:
    createCycleCountTags(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~createCycleCountTags();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sCreate();

protected slots:
    virtual void languageChange();

    virtual void sPopulateLocations();
    virtual void sParameterTypeChanged();

private:
    QButtonGroup* _codeGroup;

};

#endif // CREATECYCLECOUNTTAGS_H
