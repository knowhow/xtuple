/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGAUTHORIZEDOTNETPROCESSOR_H
#define CONFIGAUTHORIZEDOTNETPROCESSOR_H

#include "xwidget.h"

#include "configcreditcardprocessor.h"
#include "ui_configauthorizedotnetprocessor.h"

class ConfigAuthorizeDotNetProcessor : public ConfigCreditCardProcessor,
                                   public Ui::ConfigAuthorizeDotNetProcessor
{
  Q_OBJECT

  public:
    ConfigAuthorizeDotNetProcessor(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ConfigAuthorizeDotNetProcessor();

  public slots:
    virtual void sDuplicateWindow(int);
    virtual bool sSave();

  protected slots:
    virtual void languageChange();
};

#endif
