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
#include <QVariant>

#include <qmd5.h>
#include <metasql.h>

#include "crmaccount.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"

user::user(QWidget* parent, const char * name, Qt::WindowFlags fl)
  : XDialog(parent, name, fl)
{
  setupUi(this);

  _authCache     = false;
  _cUsername     = "";
  _crmacctid     = -1;
  _inTransaction = false;
  _mode          = cView;

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_crmacct, SIGNAL(clicked()),   this,     SLOT(sCrmAccount()));
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
  _grantedSite->addColumn("Granted Sites",      -1, Qt::AlignLeft);

  _locale->setType(XComboBox::Locales);

  XSqlQuery modq;
  modq.exec( "SELECT DISTINCT priv_module FROM priv ORDER BY priv_module;" );
  for (int i = 0; modq.next(); i++)
    _module->append(i, modq.value("priv_module").toString());

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
    XSqlQuery rollback("ROLLBACK;");
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

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacctid = param.toInt();

  param = pParams.value("username", &valid);
  if (valid)
    _cUsername = param.toString();

  if (! _cUsername.isEmpty() || _crmacctid > 0)
    if (! sPopulate())
      return UndefinedError;

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

  bool canEdit = (cNew == _mode || cEdit == _mode);

  _active->setEnabled(canEdit);
  _add->setEnabled(canEdit);
  _addAll->setEnabled(canEdit);
  _addGroup->setEnabled(canEdit);
//  _addSite->setEnabled(canEdit);
  _agent->setEnabled(canEdit);
  _allSites->setEnabled(canEdit);
  _email->setEnabled(canEdit);
  _employee->setReadOnly(! canEdit);
  _enhancedAuth->setEnabled(canEdit);
  _exportContents->setEnabled(canEdit);
  _initials->setEnabled(canEdit);
  _locale->setEnabled(canEdit);
  _passwd->setEnabled(canEdit);
  _properName->setEnabled(canEdit);
  _revoke->setEnabled(canEdit);
  _revokeAll->setEnabled(canEdit);
  _revokeGroup->setEnabled(canEdit);
//  _revokeSite->setEnabled(canEdit);
  _save->setEnabled(canEdit);
  _selectedSites->setEnabled(canEdit);
  _verify->setEnabled(canEdit);
  if (! canEdit)
  {
    _available->setSelectionMode(QAbstractItemView::NoSelection);
    _availableGroup->setSelectionMode(QAbstractItemView::NoSelection);
    _availableSite->setSelectionMode(QAbstractItemView::NoSelection);
    _granted->setSelectionMode(QAbstractItemView::NoSelection);
    _grantedGroup->setSelectionMode(QAbstractItemView::NoSelection);
    _grantedSite->setSelectionMode(QAbstractItemView::NoSelection);
  }

  if(canEdit)
  {
    XSqlQuery begin;
    _inTransaction = begin.exec("BEGIN;");
  }

  return NoError;
}

void user::sClose()
{
  if (_mode == cNew)
  {
    XSqlQuery delq;
    delq.prepare( "DELETE FROM usrpriv WHERE (usrpriv_username=:username);" );
    delq.bindValue(":username", _cUsername);
    delq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Privileges"),
                         delq, __FILE__, __LINE__);
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

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! username.contains(QRegExp("[A-Za-z]")), _username,
                          tr("You must enter a valid Username before you can save this User."))
         << GuiErrorCheck(_passwd->text().isEmpty(), _passwd,
                          tr("You must enter a valid Password before you can save this User."))
         << GuiErrorCheck(_passwd->text() != _verify->text(), _passwd,
                          tr("The entered password and verify do not match. "
                             "Please enter both again carefully."))
   ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot save User"), errors))
  {
     if (_passwd->text() != _verify->text())
     {
      _passwd->clear();
      _verify->clear();
     }
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

  XSqlQuery usrq;
  if (_mode == cNew)
  {
    usrq.prepare("SELECT usesysid"
                 "  FROM pg_user"
                 " WHERE (usename=:username);" );
    usrq.bindValue(":username", username);
    usrq.exec();
    if (!usrq.first())
    {
      usrq.prepare("SELECT createUser(:username, :createUsers);");
      usrq.bindValue(":username", username);
      usrq.bindValue(":createUsers", QVariant(_createUsers->isChecked()));
      usrq.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Creating User"),
                               usrq, __FILE__, __LINE__))
        return false;
    }
  }
  else if (_mode == cEdit)
  {
    if(_createUsers->isEnabled())
    {
      usrq.prepare("SELECT setUserCanCreateUsers(:username, :createUsers);");
      usrq.bindValue(":username", username);
      usrq.bindValue(":createUsers", QVariant(_createUsers->isChecked()));
      usrq.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving User"),
                               usrq, __FILE__, __LINE__))
        return false;
    }
  }

  if(_createUsers->isEnabled())
  {
    usrq.prepare("SELECT pg_has_role(:username,'xtrole','member') AS result;");
    usrq.bindValue(":username", username);
    usrq.exec();
    if(usrq.first() && !usrq.value("result").toBool())
    {
      usrq.exec( QString("ALTER GROUP xtrole ADD USER %1;")
              .arg(username) );
    }
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving User"),
                             usrq, __FILE__, __LINE__))
      return false;
  }

  if (_passwd->text() != "        ")
  {
    usrq.prepare( QString( "ALTER USER %1 WITH PASSWORD :password;")
               .arg(username) );
    usrq.bindValue(":password", passwd);
    usrq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Setting Password"),
                             usrq, __FILE__, __LINE__))
      return false;
  }

  usrq.prepare("SELECT setUserPreference(:username, 'DisableExportContents', :export),"
            "       setUserPreference(:username, 'UseEnhancedAuthentication', :enhanced),"
            "       setUserPreference(:username, 'selectedSites', :sites),"
            "       setUserPreference(:username, 'propername', :propername),"
            "       setUserPreference(:username, 'email', :email),"
            "       setUserPreference(:username, 'initials', :initials),"
            "       setUserPreference(:username, 'locale_id', text(:locale_id)),"
            "       setUserPreference(:username, 'agent', :agent),"
            "       setUserPreference(:username, 'active', :active);");
  usrq.bindValue(":username", username);
  usrq.bindValue(":export", (_exportContents->isChecked() ? "t" : "f"));
  usrq.bindValue(":enhanced", (_enhancedAuth->isChecked() ? "t" : "f"));
  usrq.bindValue(":sites", (_selectedSites->isChecked() ? "t" : "f"));
  usrq.bindValue(":propername", _properName->text());
  usrq.bindValue(":email", _email->text());
  usrq.bindValue(":initials", _initials->text());
  usrq.bindValue(":locale_id", _locale->id());
  usrq.bindValue(":agent", (_agent->isChecked() ? "t" : "f"));
  usrq.bindValue(":active", (_active->isChecked() ? "t" : "f"));
  usrq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving User"),
                           usrq, __FILE__, __LINE__))
    return false;

  omfgThis->sUserUpdated(username);
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
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Groups"),
                           groups, __FILE__, __LINE__))
    return;

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
  XSqlQuery privq;
  privq.prepare("SELECT grantPriv(:username, :priv_id) AS result;");
  privq.bindValue(":username", _cUsername);
  privq.bindValue(":priv_id", _available->id());
  privq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Granting Privilege"),
                           privq, __FILE__, __LINE__))
    return;

  sModuleSelected(_module->currentText());
}

void user::sAddAll()
{
  XSqlQuery privq;
  privq.prepare("SELECT grantAllModulePriv(:username, :module) AS result;");
  privq.bindValue(":username", _cUsername);
  privq.bindValue(":module", _module->currentText());
  privq.exec();
  if (privq.first())
  {
    int result = privq.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Granting Privileges"),
                           storedProcErrorLookup("grantAllModulePriv", result),
                           __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Granting Privileges"),
                           privq, __FILE__, __LINE__))
    return;

  sModuleSelected(_module->currentText());
}

void user::sRevoke()
{
  XSqlQuery privq;
  privq.prepare("SELECT revokePriv(:username, :priv_id) AS result;");
  privq.bindValue(":username", _cUsername);
  privq.bindValue(":priv_id", _granted->id());
  privq.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Revoking Privileges"),
                           privq, __FILE__, __LINE__))
    return;

  sModuleSelected(_module->currentText());
}

void user::sRevokeAll()
{
  XSqlQuery privq;
  privq.prepare("SELECT revokeAllModulePriv(:username, :module) AS result;");
  privq.bindValue(":username", _cUsername);
  privq.bindValue(":module", _module->currentText());
  privq.exec();
  if (privq.first())
  {
    int result = privq.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Revoking Privileges"),
                           storedProcErrorLookup("revokeAllModulePriv", result),
                           __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Revoking Privileges"),
                                privq, __FILE__, __LINE__))
    return;

  sModuleSelected(_module->currentText());
}

void user::sAddGroup()
{
  XSqlQuery grpq;
  grpq.prepare("SELECT grantGroup(:username, :grp_id) AS result;");
  grpq.bindValue(":username", _cUsername);
  grpq.bindValue(":grp_id", _availableGroup->id());
  grpq.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Granting Group Privileges"),
                                grpq, __FILE__, __LINE__))
    return;

  sModuleSelected(_module->currentText());
}

void user::sRevokeGroup()
{
  XSqlQuery grpq;
  grpq.prepare("SELECT revokeGroup(:username, :grp_id) AS result;");
  grpq.bindValue(":username", _cUsername);
  grpq.bindValue(":grp_id", _grantedGroup->id());
  grpq.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Revoking Group Privileges"),
                                grpq, __FILE__, __LINE__))
    return;

  sModuleSelected(_module->currentText());
}

void user::sCheck()
{
  _cUsername = _username->text().trimmed();
  if(omfgThis->useCloud())
    _cUsername = _cUsername + "_" + omfgThis->company();
  if (_cUsername.length() > 0)
  {
    XSqlQuery usrq;
    usrq.prepare("SELECT * FROM usr WHERE (usr_username=:username);");
    usrq.bindValue(":username", _cUsername);
    usrq.exec();
    if (usrq.first())
    {
      sPopulate();
      _mode = cEdit;
      _username->setEnabled(FALSE);
      _properName->setFocus();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting User"),
                                  usrq, __FILE__, __LINE__))
      return;
  }
}

bool user::sPopulate()
{
  XSqlQuery usrq;
  if (! _cUsername.isEmpty())
  {
    usrq.prepare("SELECT *, userCanCreateUsers(usr_username) AS createusers,"
                 "       userCanCreateUsers(getEffectiveXtUser()) AS enablecreateusers,"
                 "       crmacct_id, crmacct_emp_id, crmacct_owner_username"
                 "  FROM usr"
                 "  LEFT OUTER JOIN crmacct ON (usr_username=crmacct_usr_username) "
                 "WHERE (usr_username=:usr_username);" );
    usrq.bindValue(":usr_username", _cUsername);
  }
  else if (_crmacctid > 0)
  {
    usrq.prepare("SELECT LOWER(crmacct_number) AS usr_username,"
                 "       crmacct_name          AS usr_propername,"
                 "       (SELECT locale_id"
                 "          FROM locale"
                 "         WHERE locale_code='Default') AS usr_locale_id,"
                 "       NULL  AS usr_passwd,  cntct_initials AS usr_initials,"
                 "       FALSE AS usr_agent,   crmacct_active AS usr_active,"
                 "       NULL  AS usr_window,  cntct_email AS usr_email,"
                 "       FALSE AS createusers,"
                 "       userCanCreateUsers(getEffectiveXtUser()) AS enablecreateusers,"
                 "       crmacct_id, crmacct_emp_id, crmacct_owner_username"
                 "  FROM crmacct"
                 "  LEFT OUTER JOIN cntct ON (crmacct_cntct_id_1=cntct_id)"
                 " WHERE (crmacct_id=:id);");
    usrq.bindValue(":id", _crmacctid);
  }

  usrq.exec();
  if (usrq.first())
  {
    if(omfgThis->useCloud() && usrq.value("usr_username").toString().endsWith("_"+omfgThis->company()))
      _username->setText(usrq.value("usr_username").toString().left(usrq.value("usr_username").toString().length() - (omfgThis->company().length()+1)));
    else
      _username->setText(usrq.value("usr_username"));
    _active->setChecked(usrq.value("usr_active").toBool());
    _properName->setText(usrq.value("usr_propername"));
    _initials->setText(usrq.value("usr_initials"));
    _email->setText(usrq.value("usr_email"));
    _locale->setId(usrq.value("usr_locale_id").toInt());
    _agent->setChecked(usrq.value("usr_agent").toBool());
    _createUsers->setChecked(usrq.value("createusers").toBool());
    _createUsers->setEnabled(usrq.value("enablecreateusers").toBool());
    _employee->setId(usrq.value("crmacct_emp_id").toInt());
    _crmacctid = usrq.value("crmacct_id").toInt();
    _crmowner = usrq.value("crmacct_owner_username").toString();

    _passwd->setText("        ");
    _verify->setText("        ");

    usrq.prepare( "SELECT usrpref_value "
               "  FROM usrpref "
               " WHERE ( (usrpref_name = 'DisableExportContents') "
               "   AND (usrpref_username=:username) ); ");
    usrq.bindValue(":username", _cUsername);
    usrq.exec();
    if(usrq.first())
      _exportContents->setChecked(usrq.value("usrpref_value").toString()=="t");
    else
      _exportContents->setChecked(FALSE);

    usrq.prepare( "SELECT usrpref_value "
               "  FROM usrpref "
               " WHERE ( (usrpref_name = 'UseEnhancedAuthentication') "
               "   AND (usrpref_username=:username) ); ");
    usrq.bindValue(":username", _cUsername);
    usrq.exec();
    _authCache = false;
    if(usrq.first())
      _authCache = (usrq.value("usrpref_value").toString()=="t");
    _enhancedAuth->setChecked(_authCache);

    usrq.prepare( "SELECT priv_module "
               "FROM usrpriv, priv "
               "WHERE ( (usrpriv_priv_id=priv_id)"
               " AND (usrpriv_username=:username) ) "
               "ORDER BY priv_module "
               "LIMIT 1;" );
    usrq.bindValue(":username", _cUsername);
    usrq.exec();
    if (usrq.first())
    {
      _module->setCode(usrq.value("priv_module").toString());
      sModuleSelected(_module->currentText());
    }
    else
    {
      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting User"),
                                usrq, __FILE__, __LINE__))
    return false;

  usrq.prepare( "SELECT usrpref_value "
             "  FROM usrpref "
             " WHERE ( (usrpref_name = 'selectedSites') "
             "   AND (usrpref_username=:username) "
             "   AND (usrpref_value='t') ); ");
  usrq.bindValue(":username", _cUsername);
  usrq.exec();
  if(usrq.first())
    _selectedSites->setChecked(TRUE);
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting User Sites"),
                                usrq, __FILE__, __LINE__))
    return false;

  if (_metrics->boolean("MultiWhs"))
    populateSite();

  _crmacct->setEnabled(_crmacctid > 0 &&
                       (_privileges->check("MaintainAllCRMAccounts") ||
                        _privileges->check("ViewAllCRMAccounts") ||
                        (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                        (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));


  return true;
}

void user::sEnhancedAuthUpdate()
{
  if((_mode == cEdit) && (_authCache != _enhancedAuth->isChecked()) && (_passwd->text() == "        "))
    QMessageBox::information( this, tr("Enhanced Authentication"),
      tr("<p>You have changed this user's Enhanced Authentication option. "
         "The password must be updated in order for this change to take "
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

  XSqlQuery siteq;
  siteq.prepare("SELECT grantSite(:username, :warehous_id) AS result;");
  siteq.bindValue(":username", _cUsername);
  siteq.bindValue(":warehous_id", _availableSite->id());
  siteq.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Granting Site Privilege"),
                           siteq, __FILE__, __LINE__))
    return;

  populateSite();
}

void user::sRevokeSite()
{
  XSqlQuery siteq;
  siteq.prepare("SELECT revokeSite(:username, :warehous_id) AS result;");
  siteq.bindValue(":username", _cUsername);
  siteq.bindValue(":warehous_id", _grantedSite->id());
  siteq.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Revoking Site Privilege"),
                           siteq, __FILE__, __LINE__))
    return;

  populateSite();
}

void user::populateSite()
{
  XSqlQuery siteq;
  if (_mode == cNew)
  {
    siteq.prepare("SELECT warehous_id, warehous_code"
                  "  FROM whsinfo"
                  " ORDER BY warehous_code;");
    _availableSite->populate(siteq);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Sites"),
                             siteq, __FILE__, __LINE__))
      return;
  }
  else
  {
    QString sql;
    MetaSQLQuery mql;
    ParameterList params;

    if(omfgThis->useCloud())
      params.append("username", _username->text().trimmed().toLower() + "_" + omfgThis->company());
    else
      params.append("username", _username->text().trimmed().toLower());

    sql = "SELECT warehous_id, warehous_code "
          " FROM whsinfo "
          " WHERE warehous_id NOT IN ( "
          "     SELECT warehous_id "
          "     FROM whsinfo, usrsite "
          "     WHERE ( (usrsite_warehous_id=warehous_id) "
          "     AND (usrsite_username=<? value('username') ?>))) ";

    mql.setQuery(sql);
    siteq = mql.toQuery(params);
    _availableSite->populate(siteq);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Ungranted Sites"),
                             siteq, __FILE__, __LINE__))
      return;

    sql = "SELECT warehous_id,warehous_code,0 AS warehous_level "
          "FROM whsinfo, usrsite "
          "WHERE ( (usrsite_warehous_id=warehous_id) "
          " AND (usrsite_username=<? value('username') ?>)) ";

    mql.setQuery(sql);
    siteq = mql.toQuery(params);
    _grantedSite->populate(siteq);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Granted Sites"),
                             siteq, __FILE__, __LINE__))
      return;
  }
}

void user::done(int result)
{
  if(_inTransaction)
  {
    if(result == QDialog::Accepted)
      XSqlQuery commit("COMMIT;");
    else
      XSqlQuery rollback("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::done(result);
}

void user::sCrmAccount()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctid);
  if ((cView == _mode && _privileges->check("ViewAllCRMAccounts")) ||
      (cView == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner) ||
      (cEdit == _mode && _privileges->check("ViewAllCRMAccounts")
                      && ! _privileges->check("MaintainAllCRMAccounts")) ||
      (cEdit == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && ! _privileges->check("MaintainPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner))
    params.append("mode", "view");
  else if ((cEdit == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cEdit == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                           && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else if ((cNew == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cNew == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                          && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else
  {
    qWarning("tried to open CRM Account window without privilege");
    return;
  }

  crmaccount *newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}
