/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef USER_H
#define USER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_user.h"

class user : public XDialog, public Ui::user
{
    Q_OBJECT

public:
    user(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = 0);
    ~user();
    bool save();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual bool sPopulate();
    virtual void populateSite();
    virtual void done(int);

protected slots:
    virtual void languageChange();

    virtual void sClose();
    virtual void sCrmAccount();
    virtual void sSave();
    virtual void sModuleSelected( const QString & pModule );
    virtual void sAdd();
    virtual void sAddAll();
    virtual void sRevoke();
    virtual void sRevokeAll();
    virtual void sEnhancedAuthUpdate();
    virtual void sAddGroup();
    virtual void sRevokeGroup();
    virtual void sAddSite();
    virtual void sRevokeSite();

private:
    QString _cUsername;
    bool _authCache;
    int  _crmacctid;
    QString _crmowner;
    bool _inTransaction;
    int  _mode;

};

#endif // USER_H
