/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef USERS_H
#define USERS_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_users.h"

class users : public XWidget, public Ui::users
{
    Q_OBJECT

public:
    users(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~users();

protected slots:
    virtual void languageChange();

    virtual void sFillList();
    virtual void sNew();
    virtual void sEdit();
    virtual void sPrint();

};

#endif // USERS_H
