/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGCYBERSOURCEPROCESSOR_H
#define CONFIGCYBERSOURCEPROCESSOR_H

#include "xwidget.h"

#include "configcreditcardprocessor.h"
#include "ui_configcybersourceprocessor.h"

class ConfigCyberSourceProcessor : public ConfigCreditCardProcessor,
                                   public Ui::ConfigCyberSourceProcessor
{
  Q_OBJECT

  public:
    ConfigCyberSourceProcessor(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ConfigCyberSourceProcessor();

  public slots:
    virtual bool sSave();

  protected slots:
    virtual void languageChange();
};

#endif
