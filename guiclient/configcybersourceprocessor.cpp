/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configcybersourceprocessor.h"

#include <QMessageBox>
#include <QSqlError>

ConfigCyberSourceProcessor::ConfigCyberSourceProcessor(QWidget* parent, Qt::WFlags fl)
    : ConfigCreditCardProcessor(parent, fl)
{
  setupUi(this);
}

ConfigCyberSourceProcessor::~ConfigCyberSourceProcessor()
{
  // no need to delete child widgets, Qt does it all for us
}

void ConfigCyberSourceProcessor::languageChange()
{
  retranslateUi(this);
}

bool ConfigCyberSourceProcessor::sSave()
{
  _metrics->load();

  return true;
}
