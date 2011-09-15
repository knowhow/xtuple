/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGUREIM_H
#define CONFIGUREIM_H

#include "xabstractconfigure.h"
#include "ui_configureIM.h"

class configureIM : public XAbstractConfigure, public Ui::configureIM
{
    Q_OBJECT

public:
    configureIM(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureIM();

public slots:
    virtual bool sSave();

protected slots:
    virtual void languageChange();
    virtual void sHandleShippingFormCopies( int pValue );
    virtual void sEditShippingFormWatermark();

signals:
    void saving();
};

#endif // CONFIGUREIM_H
