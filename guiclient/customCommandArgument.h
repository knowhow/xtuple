/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CUSTOMCOMMANDARGUMENT_H
#define CUSTOMCOMMANDARGUMENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_customCommandArgument.h"

class customCommandArgument : public XDialog, public Ui::customCommandArgument
{
    Q_OBJECT

public:
    customCommandArgument(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~customCommandArgument();

    virtual SetResponse set( const ParameterList & pParams );

public slots:
    virtual void sSave();

protected:
    int _mode;
    int _cmdid;
    int _cmdargid;

    virtual void populate();

protected slots:
    virtual void languageChange();

private:

};

#endif // CUSTOMCOMMANDARGUMENT_H
