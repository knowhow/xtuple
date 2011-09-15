/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REPRIORITIZEWO_H
#define REPRIORITIZEWO_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_reprioritizeWo.h"

class reprioritizeWo : public XDialog, public Ui::reprioritizeWo
{
    Q_OBJECT

public:
    reprioritizeWo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~reprioritizeWo();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sReprioritize();
    virtual void sPopulateWoInfo(int);

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // REPRIORITIZEWO_H
