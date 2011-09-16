/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ARCHRESTORESALESHISTORY_H
#define ARCHRESTORESALESHISTORY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_archRestoreSalesHistory.h"

class archRestoreSalesHistory : public XDialog, public Ui::archRestoreSalesHistory
{
    Q_OBJECT

public:
    archRestoreSalesHistory(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~archRestoreSalesHistory();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSelect();

protected slots:
    virtual void languageChange();

private:
    int _mode;

};

#endif // ARCHRESTORESALESHISTORY_H
