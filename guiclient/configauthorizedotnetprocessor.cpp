/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configauthorizedotnetprocessor.h"

#include <QMessageBox>
#include <QSqlError>

/** \ingroup creditcards
    \class ConfigAuthorizeDotNetProcessor
    \brief Configuration UI for the Authorize.Net credit card processor
 */

ConfigAuthorizeDotNetProcessor::ConfigAuthorizeDotNetProcessor(QWidget* parent, Qt::WindowFlags fl)
    : ConfigCreditCardProcessor(parent, fl)
{
  setupUi(this);

  connect(_anDuplicateWindow, SIGNAL(valueChanged(int)), this, SLOT(sDuplicateWindow(int)));

  if (_metrics->value("CCANVer").isEmpty())
    _anVersion->setItemText(0, "3.1");
  else
    _anVersion->setItemText(0, _metrics->value("CCANVer"));
  _anDelim->setText(_metrics->value("CCANDelim"));
  _anEncap->setText(_metrics->value("CCANEncap"));
  _anDuplicateWindow->setValue(_metrics->value("CCANDuplicateWindow").toInt());

  _anMD5Hash->setText(_metrics->value("CCANMD5Hash"));
  _anMD5HashSetOnGateway->setChecked(_metrics->boolean("CCANMD5HashSetOnGateway"));
  _anMD5HashWarn->setChecked(_metrics->value("CCANMD5HashAction") == "W");
  _anMD5HashFail->setChecked(_metrics->value("CCANMD5HashAction") == "F");

  if (_metrics->value("CCANCurrency") == "TRANS")
    _anCurrTransaction->setChecked(true);
  else if (! _metrics->value("CCANCurrency").isEmpty())
  {
    _anCurrFixed->setChecked(true);
    _anCurrFixedValue->setId(_metrics->value("CCANCurrency").toInt());
  }

  _anUsingWellsFargoSecureSource->setChecked(_metrics->boolean("CCANWellsFargoSecureSource"));
  _anIgnoreSSLErrors->setChecked(_metrics->boolean("CCANIgnoreSSLErrors"));

  if (0 != _metricsenc)
    _anMD5Hash->setText(_metricsenc->value("CCANMD5Hash"));
  else
    _anMD5Hash->setEnabled(false);

  sDuplicateWindow(_anDuplicateWindow->value());
}

ConfigAuthorizeDotNetProcessor::~ConfigAuthorizeDotNetProcessor()
{
  // no need to delete child widgets, Qt does it all for us
}

void ConfigAuthorizeDotNetProcessor::languageChange()
{
  retranslateUi(this);
}

bool ConfigAuthorizeDotNetProcessor::sSave()
{
  _metrics->set("CCANVer",               _anVersion->currentText());
  _metrics->set("CCANDelim",             _anDelim->text());
  _metrics->set("CCANEncap",             _anEncap->text());
  _metrics->set("CCANDuplicateWindow",   _anDuplicateWindow->cleanText());
  _metrics->set("CCANMD5HashSetOnGateway", _anMD5HashSetOnGateway->isChecked());
  if (_anMD5HashWarn->isChecked())
    _metrics->set("CCANMD5HashAction", QString("W"));
  else if (_anMD5HashFail->isChecked())
    _metrics->set("CCANMD5HashAction", QString("F"));

  if (_anCurrFixed->isChecked())
    _metrics->set("CCANCurrency", _anCurrFixedValue->id());
  else // if (_anCurrTransaction->isChecked())
    _metrics->set("CCANCurrency", QString("TRANS"));
  _metrics->set("CCANWellsFargoSecureSource", _anUsingWellsFargoSecureSource->isChecked());
  _metrics->set("CCANMD5HashSetOnGateway", _anMD5HashSetOnGateway->isChecked());
  _metrics->set("CCANIgnoreSSLErrors",     _anIgnoreSSLErrors->isChecked());

  _metrics->load();

  if (0 != _metricsenc)
  {
    _metricsenc->set("CCANMD5Hash",     _anMD5Hash->text());
    _metricsenc->load();
  }

  return true;
}

void ConfigAuthorizeDotNetProcessor::sDuplicateWindow(int p)
{
  QTime time;
  time.addSecs(p);
  _anDuplicateWindowAsHMS->setText(time.toString("HH:mm:ss"));
}
