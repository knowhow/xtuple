/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "user.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <qmd5.h>
#include <metasql.h>

#include "storedProcErrorLookup.h"

user::user(QWidget* parent, const char * name, Qt::WindowFlags fl)
  : XDialog(parent, name, fl)
{
  _inTransaction = false;
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));
  connect(_username, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_enhancedAuth, SIGNAL(toggled(bool)), this, SLOT(sEnhancedAuthUpdate()));
  connect(_grantedGroup, SIGNAL(itemSelected(int)), this, SLOT(sRevokeGroup()));
  connect(_availableGroup, SIGNAL(itemSelected(int)), this, SLOT(sAddGroup()));
  connect(_addGroup, SIGNAL(clicked()), this, SLOT(sAddGroup()));
  connect(_revokeGroup, SIGNAL(clicked()), this, SLOT(sRevokeGroup()));
  connect(_grantedSite, SIGNAL(itemSelected(int)), this, SLOT(sRevokeSite()));
  connect(_availableSite, SIGNAL(itemSelected(int)), this, SLOT(sAddSite()));
  connect(_addSite, SIGNAL(clicked()), this, SLOT(sAddSite()));
  connect(_revokeSite, SIGNAL(clicked()), this, SLOT(sRevokeSite()));

  if(omfgThis->useCloud())
    _company->setText("_" + omfgThis->company());
  else
    _company->setText("");

  _available->addColumn("Available Privileges", -1, Qt::AlignLeft);
  _granted->addColumn("Granted Privileges", -1, Qt::AlignLeft);

  _availableGroup->addColumn("Available Roles", -1, Qt::AlignLeft);
  _grantedGroup->addColumn("Granted Roles", -1, Qt::AlignLeft);

  _availableSite->addColumn("Available Sites", -1, Qt::AlignLeft);
  _grantedSite->addColumn("Granted Sites", 	-1, Qt::AlignLeft);
  
  _locale->setType(XComboBox::Locales);

  q.exec( "SELECT DISTINCT priv_module "
          "FROM priv "
          "ORDER BY priv_module;" );
  for (int i = 0; q.next(); i++)
    _module->append(i, q.value("priv_module").toString());

  _authCache = false;
  if(_evaluation == true)
  {
    _enhancedAuth->setEnabled(false);
    _passwd->setEnabled(false);
    _verify->setEnabled(false);
  }

  if(omfgThis->useCloud())
  {
    _enhancedAuth->setChecked(true);
    _enhancedAuth->setEnabled(false);
  }
  
  if (!_metrics->boolean("MultiWhs"))
    _tab->removeTab(_tab->indexOf(_siteTab));
}

user::~user()
{
  // no need to delete child widgets, Qt does it all for us
  if(_inTransaction)
    q.exec("ROLLBACK;");
}

void user::languageChange()
{
  retranslateUi(this);
}

enum SetResponse user::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _cUsername = param.toString();
    populate();
  }
      
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));

      if (_cUsername.isEmpty())
        _username->setFocus();
      else
      {
        _username->setEnabled(false);
        if(omfgThis->useCloud() && _cUsername.endsWith("_" + omfgThis->company()))
          _username->setText(_cUsername.left(_cUsername.length() - (omfgThis->company().length()+1)));
        else
          _username->setText(_cUsername);
        _active->setFocus();
        sCheck();
      }
      if (_metrics->boolean("MultiWhs"))
        populateSite();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _username->setEnabled(FALSE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  if(cView != _mode)
    _inTransaction = q.exec("BEGIN;");

  return NoError;
}

void user::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM usrpriv "
               "WHERE (usrpriv_username=:username);" );
    q.bindValue(":username", _cUsername);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  reject();
}

void user::sSave()
{
  if(save())
    accept();
}

bool user::save()
{
  QString username = _username->text().trimmed().toLower();
  if(omfgThis->useCloud())
    username = username + "_" + omfgThis->company();
  if (!username.contains(QRegExp("[A-Za-z]")))
  {
    QMessageBox::warning( this, tr("Cannot save User"),
                          tr( "You must enter a valid Username before you can save this User." ));
    _username->setFocus();
    return false;
  }

  if (_passwd->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot save User"),
                          tr( "You must enter a valid Password before you can save this User." ));
    _passwd->setFocus();
    return false;
  }

  if (_passwd->text() != _verify->text())
  {
    QMessageBox::warning( this, tr("Password do not Match"),
                          tr( "The entered password and verify do not match\n"
                              "Please enter both again carefully." ));

    _passwd->clear();
    _verify->clear();
    _passwd->setFocus();

    return false;
  }

  QString passwd = _passwd->text();
  if(_enhancedAuth->isChecked())
  {
    if(omfgThis->useCloud())
      passwd = passwd + "cloudkey" + username;
    else
      passwd = passwd + "xTuple" + username;
    passwd = QMd5(passwd);
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT usesysid"
               "  FROM pg_user"
               " WHERE (usename=:username);" );
    q.bindValue(":username", username);
    q.exec();
    if (!q.first())
    {
      q.prepare("SELECT createUser(:username, :createUsers);");
      q.bindValue(":username", username);
      q.bindValue(":createUsers", QVariant(_createUsers->isChecked()));
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return false;
      }
    }
  }
  else if (_mode == cEdit)
  {
    if(_createUsers->isEnabled())
    {
      q.prepare("SELECT setUserCanCreateUsers(:username, :createUsers);");
      q.bindValue(":username", username);
      q.bindValue(":createUsers", QVariant(_createUsers->isChecked()));
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return false;
      }
    }
  }

  if(_createUsers->isEnabled())
  {
    q.prepare("SELECT pg_has_role(:username,'xtrole','member') AS result;");
    q.bindValue(":username", username);
    q.exec();
    if(q.first() && !q.value("result").toBool())
    {
      q.exec( QString("ALTER GROUP xtrole ADD USER %1;")
              .arg(username) );
    }
    if(q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  if (_passwd->text() != "        ")
  {
    q.prepare( QString( "ALTER USER %1 WITH PASSWORD :password;")
               .arg(username) );
    q.bindValue(":password", passwd);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  q.prepare("SELECT setUserPreference(:username, 'DisableExportContents', :export),"
            "       setUserPreference(:username, 'UseEnhancedAuthentication', :enhanced),"
            "       setUserPreference(:username, 'selectedSites', :sites),"
            "       setUserPreference(:username, 'propername', :propername),"
            "       setUserPreference(:username, 'email', :email),"
            "       setUserPreference(:username, 'initials', :initials),"
            "       setUserPreference(:username, 'locale_id', text(:locale_id)),"
            "       setUserPreference(:username, 'agent', :agent),"
            "       setUserPreference(:username, 'active', :active);");
  q.bindValue(":username", username);
  q.bindValue(":export", (_exportContents->isChecked() ? "t" : "f"));
  q.bindValue(":enhanced", (_enhancedAuth->isChecked() ? "t" : "f"));
  q.bindValue(":sites", (_selectedSites->isChecked() ? "t" : "f"));
  q.bindValue(":propername", _properName->text());
  q.bindValue(":email", _email->text());
  q.bindValue(":initials", _initials->text());
  q.bindValue(":locale_id", _locale->id());
  q.bindValue(":agent", (_agent->isChecked() ? "t" : "f"));
  q.bindValue(":active", (_active->isChecked() ? "t" : "f"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
//////////////

  return true;
}

void user::sModuleSelected(const QString &pModule)
{
  XTreeWidgetItem *granted = NULL;
  XTreeWidgetItem *available = NULL;

  _availableGroup->clear();
  _grantedGroup->clear();
  XSqlQuery groups;
  groups.prepare("SELECT grp_id, grp_name, usrgrp_id"
                 "  FROM grp LEFT OUTER JOIN usrgrp"
                 "    ON (usrgrp_grp_id=grp_id AND usrgrp_username=:username);");
  groups.bindValue(":username", _cUsername);
  groups.exec();
  while(groups.next())
  {
    if (groups.value("usrgrp_id").toInt() == 0)
      available = new XTreeWidgetItem(_availableGroup, available, groups.value("grp_id").toInt(), groups.value("grp_name"));
    else
      granted = new XTreeWidgetItem(_grantedGroup, granted, groups.value("grp_id").toInt(), groups.value("grp_name"));
  }

  _available->clear();
  _granted->clear();

  XSqlQuery privs;
  privs.prepare( "SELECT priv_id, priv_name "
                 "FROM priv "
                 "WHERE (priv_module=:priv_module) "
                 "ORDER BY priv_name;" );
  privs.bindValue(":priv_module", pModule);
  privs.exec();
  if (privs.first())
  {
    granted = NULL;
    available = NULL;

//  Insert each priv into either the available or granted list
    XSqlQuery usrpriv;
    usrpriv.prepare( "SELECT priv_id "
                     "FROM priv, usrpriv "
                     "WHERE ( (usrpriv_priv_id=priv_id)"
                     " AND (usrpriv_username=:username)"
                     " AND (priv_module=:priv_module) );" );
    usrpriv.bindValue(":username", _cUsername);
    usrpriv.bindValue(":priv_module", _module->currentText());
    usrpriv.exec();

    XSqlQuery grppriv;
    grppriv.prepare("SELECT priv_id"
                    "  FROM priv, grppriv, usrgrp"
                    " WHERE((usrgrp_grp_id=grppriv_grp_id)"
                    "   AND (grppriv_priv_id=priv_id)"
                    "   AND (usrgrp_username=:username)"
                    "   AND (priv_module=:priv_module));");
    grppriv.bindValue(":username", _cUsername);
    grppriv.bindValue(":priv_module", _module->currentText());
    grppriv.exec();

    do
    {
      if (usrpriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1 && grppriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1)
        available = new XTreeWidgetItem(_available, available, privs.value("priv_id").toInt(), privs.value("priv_name"));
      else
      {
        granted = new XTreeWidgetItem(_granted, granted, privs.value("priv_id").toInt(), privs.value("priv_name"));
        if(usrpriv.findFirst("priv_id", privs.value("priv_id").toInt()) == -1)
          granted->setTextColor(Qt::gray);
      }
    }
    while (privs.next());
  }
}

void user::sAdd()
{
  q.prepare("SELECT grantPriv(:username, :priv_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":priv_id", _available->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void user::sAddAll()
{
  q.prepare("SELECT grantAllModulePriv(:username, :module) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":module", _module->currentText());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantAllModulePriv", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void user::sRevoke()
{
  q.prepare("SELECT revokePriv(:username, :priv_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":priv_id", _granted->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void user::sRevokeAll()
{
  q.prepare("SELECT revokeAllModulePriv(:username, :module) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":module", _module->currentText());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokeAllModulePriv", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void user::sAddGroup()
{
  q.prepare("SELECT grantGroup(:username, :grp_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":grp_id", _availableGroup->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void user::sRevokeGroup()
{
  q.prepare("SELECT revokeGroup(:username, :grp_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":grp_id", _grantedGroup->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void user::sCheck()
{
  _cUsername = _username->text().trimmed();
  if(omfgThis->useCloud())
    _cUsername = _cUsername + "_" + omfgThis->company();
  if (_cUsername.length() > 0)
  {
    q.prepare( "SELECT * "
               "FROM usr "
               "WHERE (usr_username=:username);" );
    q.bindValue(":username", _cUsername);
    q.exec();
    if (q.first())
    {
      populate();
      _mode = cEdit;
      _username->setEnabled(FALSE);
      _properName->setFocus();
    }
  }
}

void user::populate()
{
  q.prepare( "SELECT *, userCanCreateUsers(usr_username) AS createusers,"
             "       userCanCreateUsers(CURRENT_USER) AS enablecreateusers,"
             "       emp_id "
             "FROM usr LEFT OUTER JOIN emp ON (usr_username=emp_username) "
             "WHERE (usr_username=:usr_username);" );
  q.bindValue(":usr_username", _cUsername);
  q.exec();
  if (q.first())
  {
    if(omfgThis->useCloud() && q.value("usr_username").toString().endsWith("_"+omfgThis->company()))
      _username->setText(q.value("usr_username").toString().left(q.value("usr_username").toString().length() - (omfgThis->company().length()+1)));
    else
      _username->setText(q.value("usr_username"));
    _active->setChecked(q.value("usr_active").toBool());
    _properName->setText(q.value("usr_propername"));
    _initials->setText(q.value("usr_initials"));
    _email->setText(q.value("usr_email"));
    _locale->setId(q.value("usr_locale_id").toInt());
    _agent->setChecked(q.value("usr_agent").toBool());
    _createUsers->setChecked(q.value("createusers").toBool());
    _createUsers->setEnabled(q.value("enablecreateusers").toBool());
    _employee->setId(q.value("emp_id").toInt());

    _passwd->setText("        ");
    _verify->setText("        ");

    q.prepare( "SELECT usrpref_value "
               "  FROM usrpref "
               " WHERE ( (usrpref_name = 'DisableExportContents') "
               "   AND (usrpref_username=:username) ); ");
    q.bindValue(":username", _cUsername);
    q.exec();
    if(q.first())
      _exportContents->setChecked(q.value("usrpref_value").toString()=="t");
    else
      _exportContents->setChecked(FALSE);

    q.prepare( "SELECT usrpref_value "
               "  FROM usrpref "
               " WHERE ( (usrpref_name = 'UseEnhancedAuthentication') "
               "   AND (usrpref_username=:username) ); ");
    q.bindValue(":username", _cUsername);
    q.exec();
    _authCache = false;
    if(q.first())
      _authCache = (q.value("usrpref_value").toString()=="t");
    _enhancedAuth->setChecked(_authCache);

    q.prepare( "SELECT priv_module "
               "FROM usrpriv, priv "
               "WHERE ( (usrpriv_priv_id=priv_id)"
               " AND (usrpriv_username=:username) ) "
               "ORDER BY priv_module "
               "LIMIT 1;" );
    q.bindValue(":username", _cUsername);
    q.exec();
    if (q.first())
    {
      _module->setCode(q.value("priv_module").toString());
      sModuleSelected(_module->currentText());
    }
    else
    {
      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
  }
  
  q.prepare( "SELECT usrpref_value "
             "  FROM usrpref "
             " WHERE ( (usrpref_name = 'selectedSites') "
             "   AND (usrpref_username=:username) "
             "   AND (usrpref_value='t') ); ");
  q.bindValue(":username", _cUsername);
  q.exec();
  if(q.first())
    _selectedSites->setChecked(TRUE);
  
  if (_metrics->boolean("MultiWhs"))
    populateSite();
}

void user::sEnhancedAuthUpdate()
{
  if((_mode == cEdit) && (_authCache != _enhancedAuth->isChecked()) && (_passwd->text() == "        "))
    QMessageBox::information( this, tr("Enhanced Authentication"),
      tr("You have changed this user's Enhanced Authentication option.\n"
         "The password must be updated in order for this change to take\n"
         "full effect.") );
}

void user::sAddSite()
{
  if(_mode == cNew)
  {
    if(!save()) return;
    else
    {
      _mode = cEdit;
      _username->setEnabled(false);
    }
  }
  
  q.prepare("SELECT grantSite(:username, :warehous_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":warehous_id", _availableSite->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  populateSite();
}

void user::sRevokeSite()
{
  q.prepare("SELECT revokeSite(:username, :warehous_id) AS result;");
  q.bindValue(":username", _cUsername);
  q.bindValue(":warehous_id", _grantedSite->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  populateSite();
}

void user::populateSite()
{

  ParameterList params;
  QString sql;
  MetaSQLQuery mql;

  if (_mode == cNew)
  {
    sql = "SELECT warehous_id, warehous_code "
          " FROM whsinfo ";
  
    mql.setQuery(sql);
    q = mql.toQuery(params);
    _availableSite->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else 
  {
    
    if(omfgThis->useCloud())
      params.append("username", _username->text().trimmed().toLower() + "_" + omfgThis->company());
    else
      params.append("username", _username->text().trimmed().toLower());

    sql = "SELECT warehous_id, warehous_code "
          " FROM whsinfo "
          " WHERE warehous_id NOT IN ( "
          "	SELECT warehous_id "
          "	FROM whsinfo, usrsite "
          "	WHERE ( (usrsite_warehous_id=warehous_id) "
          "	AND (usrsite_username=<? value(\"username\") ?>))) ";
  
    mql.setQuery(sql);
    q = mql.toQuery(params);
    _availableSite->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  
    sql = "SELECT warehous_id,warehous_code,0 AS warehous_level "
          "FROM whsinfo, usrsite "
          "WHERE ( (usrsite_warehous_id=warehous_id) "
          " AND (usrsite_username=<? value(\"username\") ?>)) ";
		
    mql.setQuery(sql); 
    q = mql.toQuery(params);
    _grantedSite->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

  }

}

void user::done(int result)
{
  if(_inTransaction)
  {
    if(result == QDialog::Accepted)
      q.exec("COMMIT;");
    else
      q.exec("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::done(result);
}

