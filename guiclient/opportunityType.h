/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITYTYPE_H
#define OPPORTUNITYTYPE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_opportunityType.h"

class opportunityType : public XDialog, public Ui::opportunityType
{
    Q_OBJECT

public:
    opportunityType(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~opportunityType();

    virtual void populate();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _optypeid;

};

#endif // OPPORTUNITYTYPE_H
