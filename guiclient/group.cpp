/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "group.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

#define DEBUG false

/* TODO: fix transaction handling in this window.
         currently only cNew is wrapped in a transaction, so the Cancel
         button really closes the window in edit mode instead of actually
         canceling the changes.
 */

group::group(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll, SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));
  connect(_revokeAll, SIGNAL(clicked()), this, SLOT(sRevokeAll()));
  connect(_module, SIGNAL(activated(const QString&)), this, SLOT(sModuleSelected(const QString&)));
  connect(_available, SIGNAL(valid(bool)), _add, SLOT(setEnabled(bool)));
  connect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
  connect(_granted, SIGNAL(valid(bool)), _revoke, SLOT(setEnabled(bool)));
  connect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));


  _available->addColumn("Available Privileges", -1, Qt::AlignLeft, true, "priv_name");
  _granted->addColumn("Granted Privileges", -1, Qt::AlignLeft, true, "priv_name");

  q.exec( "SELECT DISTINCT priv_module "
          "FROM priv "
          "ORDER BY priv_module;" );
  for (int i = 0; q.next(); i++)
    _module->append(i, q.value("priv_module").toString());

  _trapClose = false;
}

group::~group()
{
  // no need to delete child widgets, Qt does it all for us
}

void group::languageChange()
{
  retranslateUi(this);
}

enum SetResponse group::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("grp_id", &valid);
  if (valid)
  {
    _grpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      q.exec("SELECT NEXTVAL('grp_grp_id_seq') AS grp_id;");
      if (q.first())
        _grpid = q.value("grp_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _mode = cNew;
      _trapClose = true;
      q.exec("BEGIN;");
      q.prepare( "INSERT INTO grp "
                 "( grp_id, grp_name, grp_descrip)"
                 "VALUES( :grp_id, :grp_id, '' );" );
      q.bindValue(":grp_id", _grpid);
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        q.exec("ROLLBACK;");
        _trapClose = false;
        return UndefinedError;
      }

      _module->setCurrentIndex(0);
      sModuleSelected(_module->itemText(0));
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _addAll->setEnabled(FALSE);
      _revokeAll->setEnabled(FALSE);
      disconnect(_available, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));
      disconnect(_available, SIGNAL(valid(bool)), _add, SLOT(setEnabled(bool)));
      disconnect(_granted, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));
      disconnect(_granted, SIGNAL(valid(bool)), _revoke, SLOT(setEnabled(bool)));
      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

/* override reject() instead of closeEvent() because the QDialog docs
   say the Esc key calls reject and the close event cannot be ignored
 */
void group::reject()
{
  if (DEBUG)
    qDebug("group::reject() called with _trapClose = %d", _trapClose);

  if(_trapClose)
  {
    XSqlQuery endtxn;
    switch(QMessageBox::question(this, tr("Save?"),
                                 tr("<p>Do you wish to save your changes?"),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                 QMessageBox::Yes))
    {
      case QMessageBox::Yes:
        endtxn.exec("COMMIT;");
        break;

      case QMessageBox::No:
        endtxn.exec("ROLLBACK;");
        break;

      case QMessageBox::Cancel:
      default:
        break;

    }
  }
  
  XDialog::reject();
}

void group::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    q.prepare( "SELECT grp_id "
               "  FROM grp "
               " WHERE (UPPER(grp_name)=UPPER(:grp_name));" );
    q.bindValue(":grp_name", _name->text());
    q.exec();
    if (q.first())
    {
      _grpid = q.value("grp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void group::sSave()
{
  _name->setText(_name->text().trimmed().toUpper());
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Name"),
                              tr("You must enter a valid Name for this Group.") );
    _name->setFocus();
    return;
  }

  q.prepare( "UPDATE grp "
             "   SET grp_name=:grp_name,"
             "       grp_descrip=:grp_descrip "
             " WHERE(grp_id=:grp_id);" );
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":grp_name", _name->text());
  q.bindValue(":grp_descrip", _description->text().trimmed());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");
  _trapClose = false;
  _mode = cEdit;

  done(_grpid);
}

void group::sModuleSelected(const QString &pModule)
{
  XSqlQuery avail;
  avail.prepare( "SELECT priv_id, priv_name "
                 "FROM priv "
                 "WHERE ((priv_module=:priv_module) "
                 "   AND (priv_id NOT IN (SELECT grppriv_priv_id"
                 "                        FROM grppriv"
                 "                        WHERE (grppriv_grp_id=:grpid)"
                 "                       ))) "
                 "ORDER BY priv_name;" );
  avail.bindValue(":priv_module", pModule);
  avail.bindValue(":grpid", _grpid);
  avail.exec();
  _available->populate(avail);
  if (avail.lastError().type() != QSqlError::NoError)
  {
    systemError(this, avail.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery grppriv;
  grppriv.prepare( "SELECT priv_id, priv_name "
                   "FROM priv, grppriv "
                   "WHERE ((grppriv_priv_id=priv_id)"
                   "   AND (grppriv_grp_id=:grp_id)"
                   "   AND (priv_module=:priv_module))"
                   "ORDER BY priv_name;" );
  grppriv.bindValue(":grp_id", _grpid);
  grppriv.bindValue(":priv_module", _module->currentText());
  grppriv.exec();
  _granted->populate(grppriv);
  if (grppriv.lastError().type() != QSqlError::NoError)
  {
    systemError(this, grppriv.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void group::sAdd()
{
  q.prepare("SELECT grantPrivGroup(:grp_id, :priv_id) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":priv_id", _available->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantPrivGroup", result),
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

void group::sAddAll()
{
  q.prepare("SELECT grantAllModulePrivGroup(:grp_id, :module) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":module", _module->currentText());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("grantAllModulePrivGroup", result),
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

void group::sRevoke()
{
  q.prepare("SELECT revokePrivGroup(:grp_id, :priv_id) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":priv_id", _granted->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokePrivGroup", result),
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

void group::sRevokeAll()
{
  q.prepare("SELECT revokeAllModulePrivGroup(:grp_id, :module) AS result;");
  q.bindValue(":grp_id", _grpid);
  q.bindValue(":module", _module->currentText());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("revokeAllModulePrivGroup", result),
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

void group::populate()
{
  q.prepare( "SELECT grp_name, grp_descrip "
             "  FROM grp "
             " WHERE(grp_id=:grp_id);" );
  q.bindValue(":grp_id", _grpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("grp_name"));
    _description->setText(q.value("grp_descrip"));

    q.prepare( "SELECT priv_module "
               "FROM grppriv, priv "
               "WHERE ( (grppriv_priv_id=priv_id)"
               " AND (grppriv_id=:grp_id) ) "
               "ORDER BY priv_module "
               "LIMIT 1;" );
    q.bindValue(":grp_id", _grpid);
    q.exec();
    if (q.first())
    {
      for (int counter = 0; counter < _module->count(); counter++)
      {
        if (_module->itemText(counter) == q.value("priv_module").toString())
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
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
