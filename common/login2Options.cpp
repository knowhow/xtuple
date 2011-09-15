/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "login2Options.h"

#include <QVariant>

#include "xtsettings.h"
#include "dbtools.h"

login2Options::login2Options(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  setObjectName(name);
  setupUi(this);

  connect(_buttonBox2, SIGNAL(accepted()), this, SLOT(sSave()));

  _saveSettings = true;
}

login2Options::~login2Options()
{
  // no need to delete child widgets, Qt does it all for us
}

void login2Options::languageChange()
{
  retranslateUi(this);
}

void login2Options::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("databaseURL", &valid);
  if (valid)
  {
    _databaseURL = param.toString();

    QString protocol;
    QString server;
    QString database;
    QString port;
    parseDatabaseURL(_databaseURL, protocol, server, database, port);
    _server->setText(server);
    _database->setText(database);
    _port->setText(port);
  }
  
  if(pParams.inList("useEnhancedAuthentication"))
    _enhancedAuth->setChecked(TRUE);

  if(pParams.inList("requireSSL"))
    _requireSSL->setChecked(TRUE);

  if (pParams.inList("dontSaveSettings"))
    _saveSettings = false;
}

void login2Options::sSave()
{
  buildDatabaseURL(_databaseURL, "psql", _server->text(), _database->text(), _port->text());
  
  if (_saveSettings)
  {
    xtsettingsSetValue("/xTuple/_databaseURL", _databaseURL);
    xtsettingsSetValue("/xTuple/_enhancedAuthentication", (bool)_enhancedAuth->isChecked());
    xtsettingsSetValue("/xTuple/_requireSSL", (bool)_requireSSL->isChecked());
  }

  accept();
}
