/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGURECC_H
#define CONFIGURECC_H

#include <QList>

#include "guiclient.h"
#include "xabstractconfigure.h"

#include "ui_configureCC.h"

class ConfigCreditCardProcessor;

class configureCC : public XAbstractConfigure, public Ui::configureCC
{
    Q_OBJECT

public:
    configureCC(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureCC();

public slots:
    virtual void sDuplicateWindow(int);
    virtual bool sSave();

protected slots:
    virtual void languageChange();
    virtual void sCCCompanyChanged(const int);

  protected:
    QList<ConfigCreditCardProcessor*> _configcclist;

signals:
    void saving();

};

#endif // CONFIGURECC_H
