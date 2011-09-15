/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GROUP_H
#define GROUP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_group.h"

class group : public XDialog, public Ui::group
{
    Q_OBJECT

public:
    group(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~group();

public slots:
    virtual void        reject();
    virtual SetResponse set(const ParameterList &pParams);
    virtual void sCheck();
    virtual void populate();
    virtual void sSave();
    virtual void sModuleSelected(const QString &);
    virtual void sAdd();
    virtual void sAddAll();
    virtual void sRevoke();
    virtual void sRevokeAll();

protected slots:
    virtual void languageChange();

private:
    int  _grpid;
    int  _mode;
    bool _trapClose;

};

#endif // GROUP_H
