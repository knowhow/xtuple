/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureEncryption.h"

#include <QMessageBox>

configureEncryption::configureEncryption(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  if (_metricsenc == 0)
  {
    QMessageBox::critical( this, tr("Cannot Read Configuration"),
		    tr("<p>Cannot read encrypted information from database."));
  }

  _ccEncKeyName->setText(_metrics->value("CCEncKeyName"));
  _ccWinEncKey->setText(_metrics->value("CCWinEncKey"));
  _ccLinEncKey->setText(_metrics->value("CCLinEncKey"));
  _ccMacEncKey->setText(_metrics->value("CCMacEncKey"));
}

configureEncryption::~configureEncryption()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureEncryption::languageChange()
{
  retranslateUi(this);
}

bool configureEncryption::sSave()
{
  emit saving();

  _metrics->set("CCEncKeyName",      _ccEncKeyName->text());
  _metrics->set("CCWinEncKey",       _ccWinEncKey->text());
  _metrics->set("CCLinEncKey",       _ccLinEncKey->text());
  _metrics->set("CCMacEncKey",       _ccMacEncKey->text());

  _metrics->load();

  if (0 != _metricsenc)
    _metricsenc->load();

  return true;
}
