/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EMPGROUP_H
#define EMPGROUP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_empGroup.h"

class empGroup : public XDialog, public Ui::empGroup
{
    Q_OBJECT

public:
    empGroup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~empGroup();

    static  bool userHasPriv(const int = cView);
    virtual void setVisible(bool);

public slots:
    virtual bool close();
    virtual void populate();
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave(const bool = true);
    virtual void sDelete();
    virtual void sNew();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _empgrpid;
};

#endif // EMPGROUP_H
