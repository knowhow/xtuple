/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __REGISTRATIONKEY_H__
#define __REGISTRATIONKEY_H__

#include "xabstractconfigure.h"

#include "ui_registrationKey.h"

class registrationKey : public XAbstractConfigure, public Ui::registrationKey
{
    Q_OBJECT

public:
    registrationKey(QWidget* parent = 0, const char * = 0, Qt::WFlags fl = Qt::Window);

public slots:
    virtual bool sSave();
    virtual void sCheck();

protected slots:
    virtual void languageChange();

};

#endif // __REGISTRATIONKEY_H__
