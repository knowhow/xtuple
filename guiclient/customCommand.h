/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CUSTOMCOMMAND_H
#define CUSTOMCOMMAND_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_customCommand.h"

class customCommand : public XDialog, public Ui::customCommand
{
    Q_OBJECT

public:
    customCommand(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~customCommand();

    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();

public slots:
    virtual void reject();
    virtual void sSave();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();

protected:
    int _cmdid;
    int _mode;
    QString _oldPrivname;
    bool _saved;

    bool save();

protected slots:
    virtual void languageChange();
    virtual void setMode(const int);

};

#endif // CUSTOMCOMMAND_H
