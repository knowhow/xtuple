/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef MASSEXPIRECOMPONENT_H
#define MASSEXPIRECOMPONENT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_massExpireComponent.h"

class massExpireComponent : public XWidget, public Ui::massExpireComponent
{
    Q_OBJECT

public:
    massExpireComponent(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~massExpireComponent();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sExpire();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // MASSEXPIRECOMPONENT_H
