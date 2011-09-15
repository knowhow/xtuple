/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FIXACL_H
#define FIXACL_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_fixACL.h"

class fixACL : public XWidget, public Ui::fixACL
{
    Q_OBJECT

public:
    fixACL(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = Qt::Window);
    ~fixACL();
    static bool userHasPriv(const int = 0);

public slots:
    virtual void sFix();

protected slots:
    virtual void languageChange();
};

#endif // FIXACL_H
