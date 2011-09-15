/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COMMENTTYPE_H
#define COMMENTTYPE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_commentType.h"

class commentType : public XDialog, public Ui::commentType
{
    Q_OBJECT

public:
    commentType(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~commentType();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCheck();
    virtual void sModuleSelected( const QString & pModule );
    virtual void sAdd();
    virtual void sAddAll();
    virtual void sRevoke();
    virtual void sRevokeAll();


private:
    int _cmnttypeid;
    int _mode;
};

#endif // COMMENTTYPE_H
