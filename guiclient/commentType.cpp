/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "commentType.h"

#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "storedProcErrorLookup.h"

commentType::commentType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));

  _available->addColumn("Available Sources", -1, Qt::AlignLeft);
  _granted->addColumn("Granted Sources", -1, Qt::AlignLeft);
  
  q.exec( "SELECT DISTINCT source_module "
          "FROM source "
          "ORDER BY source_module;" );
  for (int i = 0; q.next(); i++)
    _module->insertItem(i, q.value("source_module").toString());
}

commentType::~commentType()
{
  // no need to delete child widgets, Qt does it all for us
}

void commentType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse commentType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cmnttype_id", &valid);
  if (valid)
  {
    _cmnttypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      q.exec("SELECT NEXTVAL('cmnttype_cmnttype_id_seq') AS cmnttype_id");
      if (q.first())
        _cmnttypeid = q.value("cmnttype_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
      }

      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _editable->setEnabled(FALSE);
      _order->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void commentType::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Comment Type"),
                              tr("You must enter a valid Comment Type before saving this Item Type.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO cmnttype "
               "( cmnttype_id, cmnttype_name, cmnttype_descrip, cmnttype_editable, cmnttype_order ) "
               "VALUES "
               "( :cmnttype_id, :cmnttype_name, :cmnttype_descrip, :cmnttype_editable, :cmnttype_order );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE cmnttype "
               "SET cmnttype_name=:cmnttype_name,"
               "    cmnttype_descrip=:cmnttype_descrip,"
               "    cmnttype_editable=:cmnttype_editable,"
               "    cmnttype_order=:cmnttype_order "
               "WHERE (cmnttype_id=:cmnttype_id);" );

  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":cmnttype_name", _name->text());
  q.bindValue(":cmnttype_descrip", _description->text());
  q.bindValue(":cmnttype_editable", _editable->isChecked());
  q.bindValue(":cmnttype_order", _order->value());
  q.exec();

  done(_cmnttypeid);
}

void commentType::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT cmnttype_id "
               "FROM cmnttype "
               "WHERE (UPPER(cmnttype_name)=UPPER(:cmnttype_name));" );
    q.bindValue(":cmnttype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _cmnttypeid = q.value("cmnttype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void commentType::populate()
{
  q.prepare( "SELECT * "
             "FROM cmnttype "
             "WHERE (cmnttype_id=:cmnttype_id);" );
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("cmnttype_name"));
    _description->setText(q.value("cmnttype_descrip"));
    _editable->setChecked(q.value("cmnttype_editable").toBool());
    _order->setValue(q.value("cmnttype_order").toInt());
    if(q.value("cmnttype_sys").toBool())
    {
      _name->setEnabled(false);
      if(_name->text() == "ChangeLog")
        _editable->setEnabled(false);
    }
    
    q.prepare( "SELECT source_module "
               "FROM cmnttypesource, source "
               "WHERE ( (cmnttypesource_source_id=source_id)"
               " AND (cmnttypesource_cmnttype_id=:cmnttype_id) ) "
               "ORDER BY source_module "
               "LIMIT 1;" );
    q.bindValue(":cmnttype_id", _cmnttypeid);
    q.exec();
    if (q.first())
    {
      for (int counter = 0; counter < _module->count(); counter++)
      {
        if (_module->itemText(counter) == q.value("source_module").toString())
        {
          _module->setCurrentIndex(counter);
          sModuleSelected(_module->itemText(counter));
        }
      }
    }
    else
    {
      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
    }
  }
}

void commentType::sModuleSelected(const QString &pModule)
{
  XTreeWidgetItem *granted = NULL;
  XTreeWidgetItem *available = NULL;

  _available->clear();
  _granted->clear();

  XSqlQuery sources;
  sources.prepare( "SELECT source_id, source_descrip "
                   "FROM source "
                   "WHERE (source_module=:source_module) "
                   "ORDER BY source_descrip;" );
  sources.bindValue(":source_module", pModule);
  sources.exec();
  if (sources.first())
  {
    granted = NULL;
    available = NULL;

//  Insert each source into either the available or granted list
    XSqlQuery cmnttypesource;
    cmnttypesource.prepare( "SELECT source_id "
                            "FROM source, cmnttypesource "
                            "WHERE ( (cmnttypesource_source_id=source_id)"
                            " AND (cmnttypesource_cmnttype_id=:cmnttype_id)"
                            " AND (source_module=:source_module) );" );
    cmnttypesource.bindValue(":cmnttype_id", _cmnttypeid);
    cmnttypesource.bindValue(":source_module", _module->currentText());
    cmnttypesource.exec();

    do
    {
      if (cmnttypesource.findFirst("source_id", sources.value("source_id").toInt()) == -1)
        available = new XTreeWidgetItem(_available, available, sources.value("source_id").toInt(), sources.value("source_descrip"));
      else
      {
        granted = new XTreeWidgetItem(_granted, granted, sources.value("source_id").toInt(), sources.value("source_descrip"));
      }
    }
    while (sources.next());
  }
}

void commentType::sAdd()
{
  q.prepare("SELECT grantCmnttypeSource(:cmnttype_id, :source_id) AS result;");
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":source_id", _available->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void commentType::sAddAll()
{
  q.prepare("SELECT grantAllModuleCmnttypeSource(:cmnttype_id, :module) AS result;");
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":module", _module->currentText());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantAllModuleCmnttypeSource", result),
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

void commentType::sRevoke()
{
  q.prepare("SELECT revokeCmnttypeSource(:cmnttype_id, :source_id) AS result;");
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":source_id", _granted->id());
  q.exec();
  // no storedProcErrorLookup because the function returns bool, not int
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sModuleSelected(_module->currentText());
}

void commentType::sRevokeAll()
{
  q.prepare("SELECT revokeAllModuleCmnttypeSource(:cmnttype_id, :module) AS result;");
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":module", _module->currentText());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokeAllModuleCmnttypeSource", result),
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

