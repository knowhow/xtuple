/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "userPreferences.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QSqlError>
#include <QFile>

#include <qmd5.h>
#include "storedProcErrorLookup.h"

#include <parameter.h>

#include "hotkey.h"
#include "imageList.h"
#include "timeoutHandler.h"

extern QString __password;

userPreferences::userPreferences(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _pref = _preferences;
  _altPref = 0;

  if(!_privileges->check("MaintainPreferencesOthers"))
    _selectedUser->setEnabled(false);

  QPushButton* apply = _buttonBox->button(QDialogButtonBox::Apply);
  connect(apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_buttonBox,         SIGNAL(rejected()),     this, SLOT(sClose()));
  connect(_buttonBox,          SIGNAL(accepted()),     this, SLOT(sSave()));

  connect(_backgroundList,SIGNAL(clicked()),     this, SLOT(sBackgroundList()));
  connect(_selectedUser,  SIGNAL(toggled(bool)), this, SLOT(sPopulate()));
  connect(_user,          SIGNAL(newID(int)),    this, SLOT(sPopulate()));
    //hot key signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_hotkey, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_hotkey, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_hotkey, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

  _hotkey->addColumn(tr("Keystroke"), _itemColumn, Qt::AlignLeft );
  _hotkey->addColumn(tr("Action"),    -1,          Qt::AlignLeft );
  _hotkey->addColumn("key",           0,           Qt::AlignLeft );

  connect(_warehouses, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(sWarehouseToggled(QTreeWidgetItem*)));
  connect(_event, SIGNAL(itemSelected(int)), this, SLOT(sAllWarehousesToggled(int)));
  connect(_event, SIGNAL(itemSelectionChanged()), this, SLOT(sFillWarehouseList()));

  _event->addColumn(tr("Module"),      50,   Qt::AlignCenter, true,  "evnttype_module" );
  _event->addColumn(tr("Name"),        150,  Qt::AlignLeft,   true,  "evnttype_name"   );
  _event->addColumn(tr("Description"), -1,   Qt::AlignLeft,   true,  "evnttype_descrip"   );
  _event->populate( "SELECT evnttype_id, evnttype_module, evnttype_name, evnttype_descrip "
                    "FROM evnttype "
                    "ORDER BY evnttype_module, evnttype_name" );

  _warehouses->addColumn(tr("Notify"),      50,         Qt::AlignCenter, true,  "notify" );
  _warehouses->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter, true,  "warehous_code" );
  _warehouses->addColumn(tr("Description"), -1,         Qt::AlignLeft,   true,  "warehous_descrip"   );
  _warehouses->populate( "SELECT warehous_id, TEXT('-') AS notify, warehous_code, warehous_descrip "
                        "FROM warehous "
                        "ORDER BY warehous_code" );

  _dirty = FALSE;

#ifndef Q_WS_MAC
  _backgroundList->setMaximumWidth(25);
#endif

  _user->setType(XComboBox::Users);

  _ellipsesAction->append(1, tr("List"));
  _ellipsesAction->append(2, tr("Search"));

  if (!_metrics->boolean("EnableBatchManager"))
  {
    _alarmEmail->setVisible(false);
    _emailEvents->setVisible(false);
  }

  sPopulate();
  adjustSize();
}

userPreferences::~userPreferences()
{
  // no need to delete child widgets, Qt does it all for us
  if(_altPref)
    delete _altPref;
}

void userPreferences::languageChange()
{
  retranslateUi(this);
}

void userPreferences::setBackgroundImage(int pImageid)
{
  q.prepare( "SELECT image_id, (image_name || ' - ' || image_descrip) AS description "
             "FROM image "
             "WHERE (image_id=:image_id);" );
  q.bindValue(":image_id", pImageid);
  q.exec();
  if (q.first())
  {
    _backgroundImageid = q.value("image_id").toInt();
    _background->setText(q.value("description").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void userPreferences::sBackgroundList()
{
  ParameterList params;
  params.append("image_id", _backgroundImageid);

  imageList newdlg(this, "", TRUE);
  newdlg.set(params);

  int imageid;
  if ((imageid = newdlg.exec()) != 0)
    setBackgroundImage(imageid);
}

void userPreferences::sPopulate()
{
  if (_currentUser->isChecked())
  {
    _pref = _preferences;
    _username->setText(omfgThis->username());
  }
  else
  {
    if(_altPref)
      delete _altPref;
    _altPref = new Preferences(_user->currentText());
    _pref = _altPref;
    _username->setText(_user->currentText());
  }
  _username->setEnabled(FALSE);
  _currentpassword->setEnabled(_currentUser->isChecked());
  _newpassword->setEnabled(_currentUser->isChecked());
  _retypepassword->setEnabled(_currentUser->isChecked());
  

  if (_pref->value("BackgroundImageid").toInt() > 0)
  {
    _backgroundImage->setChecked(TRUE);
    setBackgroundImage(_pref->value("BackgroundImageid").toInt());
  }
  else
  {
    _noBackgroundImage->setChecked(TRUE);
    _background->clear();
    _backgroundImageid = -1;
  }

  if (_pref->value("PreferredWarehouse").toInt() == -1)
    _noWarehouse->setChecked(TRUE);
  else
  {
    _selectedWarehouse->setChecked(TRUE);
    _warehouse->setId(_pref->value("PreferredWarehouse").toInt());
  }

  if (_pref->value("InterfaceWindowOption") == "Workspace")
    _interfaceWorkspace->setChecked(true);
  else
    _interfaceTopLevel->setChecked(true);
    
  if (_pref->boolean("CopyListsPlainText"))
    _plainText->setChecked(true);
  else
    _richText->setChecked(true);

  _enableSpell->setChecked(_pref->boolean("SpellCheck"));

  //_rememberCheckBoxes->setChecked(! _pref->boolean("XCheckBox/forgetful"));
  
  _inventoryMenu->setChecked(_pref->boolean("ShowIMMenu"));
  _productsMenu->setChecked(_pref->boolean("ShowPDMenu"));
  _scheduleMenu->setChecked(_pref->boolean("ShowMSMenu"));
  _manufactureMenu->setChecked(_pref->boolean("ShowWOMenu"));
  _crmMenu2->setChecked(_pref->boolean("ShowCRMMenu"));
  _purchaseMenu->setChecked(_pref->boolean("ShowPOMenu"));
  _salesMenu->setChecked(_pref->boolean("ShowSOMenu"));
  _accountingMenu->setChecked(_pref->boolean("ShowGLMenu"));

  _inventoryToolbar->setChecked(_pref->boolean("ShowIMToolbar"));
  _productsToolbar->setChecked(_pref->boolean("ShowPDToolbar"));
  _scheduleToolbar->setChecked(_pref->boolean("ShowMSToolbar"));
  _manufactureToolbar->setChecked(_pref->boolean("ShowWOToolbar"));
  _crmToolbar2->setChecked(_pref->boolean("ShowCRMToolbar"));
  _purchaseToolbar->setChecked(_pref->boolean("ShowPOToolbar"));
  _salesToolbar->setChecked(_pref->boolean("ShowSOToolbar"));
  _accountingToolbar->setChecked(_pref->boolean("ShowGLToolbar"));
  
  _listNumericItemsFirst->setChecked(_pref->boolean("ListNumericItemNumbersFirst"));
  _ignoreTranslation->setChecked(_pref->boolean("IngoreMissingTranslationFiles"));

  _idleTimeout->setValue(_pref->value("IdleTimeout").toInt());

  _emailEvents->setChecked(_pref->boolean("EmailEvents"));

  _alarmEvent->setChecked(_pref->boolean("AlarmEventDefault"));
  _alarmEmail->setChecked(_pref->boolean("AlarmEmailDefault"));
  _alarmSysmsg->setChecked(_pref->boolean("AlarmSysmsgDefault"));

  if(_pref->value("DefaultEllipsesAction") == "search")
    _ellipsesAction->setId(2);
  else
    _ellipsesAction->setId(1);

  _alternating->setChecked(!_pref->boolean("NoAlternatingRowColors"));
    
  //Hide for PostBooks 
  if (_metrics->value("Application") == "PostBooks")
  {
    _scheduleMenu->hide();
    _scheduleToolbar->hide();
  }
  
  if (!_metrics->boolean("MultiWhs"))
    _warehouseGroup->hide();
    
  _debug->setChecked(_pref->boolean("EnableScriptDebug"));

  sFillList();
  sFillWarehouseList();
}

void userPreferences::sApply()
{
  sSave(false);
  sPopulate();
}

void userPreferences::sSave(bool close)
{
  if (_backgroundImage->isChecked())
    _pref->set("BackgroundImageid", _backgroundImageid);
  else
    _pref->set("BackgroundImageid", -1);

  _pref->set("SpellCheck", _enableSpell->isChecked());

  if(_enableSpell->isChecked())
  {
      QString langName = QLocale::languageToString(QLocale().language());
      QString appPath = QApplication::applicationDirPath();
      QString fullPathWithoutExt = appPath + "/" + langName;
      QFile affFile(fullPathWithoutExt + ".aff");
      QFile dicFile(fullPathWithoutExt + ".dic");
      if(!affFile.exists() || !dicFile.exists())
      {
        QMessageBox::warning( this, tr("Spell Dictionary Missing"),
                   tr("The following Hunspell files are required for spell checking: <p>")
                   + fullPathWithoutExt + tr(".aff <p>") + fullPathWithoutExt + tr(".dic"));
      }
  }
  
  _pref->set("ShowIMMenu", _inventoryMenu->isChecked());
  _pref->set("ShowPDMenu", _productsMenu->isChecked());
  _pref->set("ShowMSMenu", _scheduleMenu->isChecked());
  _pref->set("ShowWOMenu", _manufactureMenu->isChecked());
  _pref->set("ShowCRMMenu", _crmMenu2->isChecked());
  _pref->set("ShowPOMenu", _purchaseMenu->isChecked());
  _pref->set("ShowSOMenu", _salesMenu->isChecked());
  _pref->set("ShowGLMenu", _accountingMenu->isChecked());
 
  _pref->set("ShowIMToolbar", _inventoryToolbar->isChecked());
  _pref->set("ShowPDToolbar", _productsToolbar->isChecked());
  _pref->set("ShowMSToolbar", _scheduleToolbar->isChecked());
  _pref->set("ShowWOToolbar", _manufactureToolbar->isChecked());
  _pref->set("ShowCRMToolbar", _crmToolbar2->isChecked());
  _pref->set("ShowPOToolbar", _purchaseToolbar->isChecked());
  _pref->set("ShowSOToolbar", _salesToolbar->isChecked());
  _pref->set("ShowGLToolbar", _accountingToolbar->isChecked());
  
  _pref->set("PreferredWarehouse", ((_noWarehouse->isChecked()) ? -1 : _warehouse->id())  );
 
  _pref->set("ListNumericItemNumbersFirst", _listNumericItemsFirst->isChecked());
  _pref->set("IngoreMissingTranslationFiles", _ignoreTranslation->isChecked());

  _pref->set("IdleTimeout", _idleTimeout->value());
  omfgThis->_timeoutHandler->setIdleMinutes(_idleTimeout->value());

  if(_ellipsesAction->id() == 2)
    _pref->set("DefaultEllipsesAction", QString("search"));
  else
    _pref->set("DefaultEllipsesAction", QString("list"));

  _pref->set("NoAlternatingRowColors", !_alternating->isChecked());

  if(_interfaceWorkspace->isChecked())
    _pref->set("InterfaceWindowOption", QString("Workspace"));
  else
    _pref->set("InterfaceWindowOption", QString("TopLevel"));
    
  _pref->set("CopyListsPlainText", _plainText->isChecked());
  _pref->set("EmailEvents", _emailEvents->isChecked());

  _pref->set("AlarmEventDefault", _alarmEvent->isChecked());
  _pref->set("AlarmEmailDefault", _alarmEmail->isChecked());
  _pref->set("AlarmSysmsgDefault", _alarmSysmsg->isChecked());

  _pref->set("EnableScriptDebug", _debug->isChecked());

  if (_currentUser->isChecked())
  {
    _preferences->load();
    omfgThis->initMenuBar();
  }

  if (_currentUser->isChecked() && !_currentpassword->text().isEmpty())
  {
    if (save() && close)
     accept(); 
  }
  else if (close)
    accept();
}

bool userPreferences::save()
{
  
  if (_currentpassword->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot save User"),
                          tr( "You must enter a valid Current Password before you can save this User." ));
    _currentpassword->setFocus();
    return false;
  }

  if (_newpassword->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot save User"),
                          tr( "You must enter a valid Password before you can save this User." ));
    _newpassword->setFocus();
    return false;
  }

  QString passwd = _newpassword->text();
  QString currentpasswd = _currentpassword->text();
   
  // TODO: have to compare this against something usefull
  if(currentpasswd != __password)
  {
    QMessageBox::warning( this, tr("Cannot save User"),
                  tr( "Please Verify Current Password." ));
    _currentpassword->setFocus();
    return false;
  }

  if (_newpassword->text() != _retypepassword->text())
  {
    QMessageBox::warning( this, tr("Password do not Match"),
                   tr( "The entered password and verify do not match\n"
                       "Please enter both again carefully." ));

    _newpassword->clear();
    _retypepassword->clear();
    _newpassword->setFocus();
    return false;
  }   

  if (_newpassword->text() != "        ")
  {
    q.prepare( "SELECT usrpref_value "
                "  FROM usrpref "
                " WHERE ( (usrpref_name = 'UseEnhancedAuthentication') "
                "   AND (usrpref_username=:username) ); ");
    q.bindValue(":username", _username->text().trimmed().toLower());         
    q.exec();
    if(q.first())
    {
      if (q.value("usrpref_value").toString()=="t")
      {
        if(omfgThis->useCloud())
          passwd = passwd + "cloudkey" + _username->text();
        else
          passwd = passwd + "xTuple" + _username->text();
        passwd = QMd5(passwd);
      }
    }

    q.prepare( QString( "ALTER USER %1 WITH PASSWORD :password;")
           .arg(_username->text()) );
    q.bindValue(":password", passwd);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
  return true;
}

void userPreferences::sClose()
{
  if (_dirty)
    omfgThis->initMenuBar();

  reject();
}

void userPreferences::sFillList()
{
  _hotkey->clear();

  QString hotkey;
  QString action;
  char    key;

  XTreeWidgetItem *last = 0;
  if (_currentUser->isChecked())
  {
    for (key = '1'; key <= '9'; key++)
    {
      hotkey = QString("F%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(tr("F%1").arg(key)), action, hotkey);
    }

    for (key = 'A'; key <= 'Z'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }

    for (key = '0'; key <= '9'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = _preferences->value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }
  }
  else
  {
    Preferences pref(_user->currentText());

    for (key = '1'; key <= '9'; key++)
    {
      hotkey = QString("F%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(tr("F%1").arg(key)), action, hotkey);
    }

    for (key = 'A'; key <= 'Z'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1,QVariant( QString("Ctrl+%1").arg(key)), action, hotkey);
    }

    for (key = '0'; key <= '9'; key++)
    {
      hotkey = QString("C%1").arg(key);
      action = pref.value(hotkey);
      if (!action.isNull())
        last = new XTreeWidgetItem(_hotkey, last, -1, QVariant(QString("Ctrl+%1").arg(key)), action, hotkey);
    }
  }
}

void userPreferences::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  if (_currentUser->isChecked())
    params.append("currentUser");
  else
    params.append("username", _user->currentText());

  hotkey newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
  {
    _dirty = TRUE;
    sFillList();
  }
}

void userPreferences::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("hotkey", _hotkey->currentItem()->text(2));

  if (_currentUser->isChecked())
    params.append("currentUser");
  else
    params.append("username", _user->currentText());

  hotkey newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
  {
    _dirty = TRUE;
    sFillList();
  }
}

void userPreferences::sDelete()
{
  if (_currentUser->isChecked())
  {
    _preferences->remove(_hotkey->currentItem()->text(2));
    _preferences->load();
  }
  else
  {
    q.prepare("SELECT deleteUserPreference(:username, :name) AS _result;");
    if (_currentUser->isChecked())
      q.bindValue(":username", omfgThis->username());
    else
      q.bindValue(":username", _user->currentText());
    q.bindValue(":name", _hotkey->currentItem()->text(2));
    q.exec();
  }

  _dirty = TRUE;
  sFillList();
}

void userPreferences::sAllWarehousesToggled(int pEvnttypeid)
{
  if(!_warehouses->topLevelItemCount() > 0)
    return;

  if (_warehouses->topLevelItem(0)->text(0) == tr("Yes"))
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id) );" );
  else
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id) ); "
               "INSERT INTO evntnot "
               "(evntnot_username, evntnot_evnttype_id, evntnot_warehous_id) "
               "SELECT :username, :evnttype_id, warehous_id "
               "FROM warehous;" );

  if (_currentUser->isChecked())
	q.bindValue(":username", omfgThis->username());
  else
    q.bindValue(":username", _user->currentText());
  q.bindValue(":evnttype_id", pEvnttypeid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillWarehouseList();
}

void userPreferences::sWarehouseToggled(QTreeWidgetItem *selected)
{
  if(!selected)
    return;

  if (selected->text(0) == tr("Yes"))
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id)"
               " AND (evntnot_warehous_id=:warehous_id) );" );
  else
    q.prepare( "INSERT INTO evntnot "
               "(evntnot_username, evntnot_evnttype_id, evntnot_warehous_id) "
               "VALUES "
               "(:username, :evnttype_id, :warehous_id);" );

  if (_currentUser->isChecked())
	q.bindValue(":username", omfgThis->username());
  else
    q.bindValue(":username", _user->currentText());
  q.bindValue(":evnttype_id", _event->id());
  q.bindValue(":warehous_id", ((XTreeWidgetItem *)selected)->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillWarehouseList();
}

void userPreferences::sFillWarehouseList()
{
  for (int i = 0; i < _warehouses->topLevelItemCount(); i++)
  {
    q.prepare( "SELECT evntnot_id "
               "FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_warehous_id=:warehous_id)"
               " AND (evntnot_evnttype_id=:evnttype_id) );" );
    if (_currentUser->isChecked())
	  q.bindValue(":username", omfgThis->username());
    else
      q.bindValue(":username", _user->currentText());
    q.bindValue(":warehous_id", ((XTreeWidgetItem *)(_warehouses->topLevelItem(i)))->id());
    q.bindValue(":evnttype_id", _event->id());
    q.exec();
    if (q.first())
      _warehouses->topLevelItem(i)->setText(0, tr("Yes"));
    else
      _warehouses->topLevelItem(i)->setText(0, tr("No"));
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
