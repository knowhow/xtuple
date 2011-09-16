/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

todoItem::todoItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  connect(_buttonBox,	SIGNAL(rejected()),	this,	SLOT(sClose()));
  connect(_incident,	SIGNAL(newId(int)),	this,	SLOT(sHandleIncident()));
  connect(_buttonBox,	SIGNAL(accepted()),	this,	SLOT(sSave()));

  _started->setAllowNullDate(true);
  _due->setAllowNullDate(true);
  _assigned->setAllowNullDate(true);
  _completed->setAllowNullDate(true);
  _priority->setType(XComboBox::IncidentPriority);

  _assignedTo->setUsername(omfgThis->username());
  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);
  _assignedTo->setType(UsernameLineEdit::UsersActive);
}

void todoItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse todoItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
    _assignedTo->setUsername(param.toString());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('todoitem_todoitem_id_seq') AS todoitem_id");
      if (q.first())
      {
        _todoitemid = q.value("todoitem_id").toInt();
        _alarms->setId(_todoitemid);
	_comments->setId(_todoitemid);
        _documents->setId(_todoitemid);
        _recurring->setParent(_todoitemid, "TODO");
      }

      _name->setFocus();
      _assignedTo->setEnabled(_privileges->check("MaintainAllToDoItems") &&
                          _privileges->check("ReassignToDoItems"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _name->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _ophead->setEnabled(FALSE);
      _assigned->setEnabled(FALSE);
      _due->setEnabled(FALSE);
      _assignedTo->setEnabled(_privileges->check("MaintainAllToDoItems") &&
                            _privileges->check("ReassignToDoItems"));
      _description->setEnabled(FALSE);

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _owner->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _priority->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _ophead->setEnabled(FALSE);
      _started->setEnabled(FALSE);
      _assigned->setEnabled(FALSE);
      _due->setEnabled(FALSE);
      _completed->setEnabled(FALSE);
      _pending->setEnabled(FALSE);
      _deferred->setEnabled(FALSE);
      _neither->setEnabled(FALSE);
      _assignedTo->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _alarms->setReadOnly(TRUE);
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      _crmacct->setReadOnly(true);
      _cntct->setReadOnly(true);

      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  param = pParams.value("incdt_id", &valid);
  if (valid)
    _incident->setId(param.toInt());

  param = pParams.value("priority_id", &valid);
  if (valid)
    _priority->setId(param.toInt());

  param = pParams.value("ophead_id", &valid);
  if (valid)
    _ophead->setId(param.toInt());

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
    _incident->setExtraClause(QString(" (incdt_crmacct_id=%1) ")
				.arg(_crmacct->id()));
    _ophead->setExtraClause(QString(" (ophead_crmacct_id=%1) ")
                                .arg(_crmacct->id()));
  }

  param = pParams.value("todoitem_id", &valid);
  if (valid)
  {
    _todoitemid = param.toInt();
    sPopulate();
  }

  return NoError;
}

void todoItem::sSave()
{
  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return;

  QString storedProc;
  XSqlQuery beginq("BEGIN;");
  XSqlQuery rollbackq;
  rollbackq.prepare("ROLLBACK;");
  if (_mode == cNew)
  {
    q.prepare( "SELECT createTodoItem(:todoitem_id, :username, :name, :description, "
	       "  :incdt_id, :crmacct_id, :ophead_id, :started, :due, :status, "
               "  :assigned, :completed, :priority, :notes, :owner, :cntct_id) AS result;");
    storedProc = "createTodoItem";
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT updateTodoItem(:todoitem_id, "
	       "  :username, :name, :description, "
	       "  :incdt_id, :crmacct_id, :ophead_id, :started, :due, :status, "
               "  :assigned, :completed, :priority, :notes, :active, :owner, :cntct_id) AS result;");
    storedProc = "updateTodoItem";
  }
  q.bindValue(":todoitem_id", _todoitemid);
  q.bindValue(":owner", _owner->username());
  q.bindValue(":username",   _assignedTo->username());
  if(_assigned->date().isValid())
    q.bindValue(":assigned", _assigned->date());
  q.bindValue(":name",		_name->text());
  q.bindValue(":description",	_description->text());
  if (_incident->id() > 0)
    q.bindValue(":incdt_id",	_incident->id());	// else NULL
  if (_crmacct->id() > 0)
    q.bindValue(":crmacct_id",	_crmacct->id());	// else NULL
  q.bindValue(":started",	_started->date());
  q.bindValue(":due",		_due->date());
  q.bindValue(":assigned",	_assigned->date());
  q.bindValue(":completed",	_completed->date());
  if(_priority->isValid())
    q.bindValue(":priority", _priority->id());
  q.bindValue(":notes",		_notes->toPlainText());
  q.bindValue(":active",	QVariant(_active->isChecked()));
  if(_ophead->id() > 0)
    q.bindValue(":ophead_id", _ophead->id());

  QString status;
  if (_completed->date().isValid())
    status = "C";
  else if (_deferred->isChecked())
    status = "D";
  else if (_pending->isChecked())
    status = "P";
  else if (_started->date().isValid())
    status = "I";
  else
    status = "N";
  q.bindValue(":status", status);

  if (_cntct->isValid())
    q.bindValue(":cntct_id", _cntct->id());

  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup(storedProc, result), __FILE__, __LINE__);
      rollbackq.exec();
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollbackq.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // TODO: make this part of {create,update}TodoItem?
  if (_recurring->isRecurring())
  {
    XSqlQuery recurq;
    recurq.prepare("UPDATE todoitem"
                   "   SET todoitem_recurring_todoitem_id=:parent_id"
                   " WHERE todoitem_id=:id;");
    recurq.bindValue(":parent_id", _recurring->parentId());
    recurq.bindValue(":id",        _todoitemid);
    if (! recurq.exec())
    {
      rollbackq.exec();
      systemError(this, recurq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    XSqlQuery recurq;
    recurq.prepare("UPDATE todoitem"
                   "   SET todoitem_recurring_todoitem_id=NULL"
                   " WHERE todoitem_id=:id;");
    recurq.bindValue(":id",        _todoitemid);
    if (! recurq.exec())
    {
      rollbackq.exec();
      systemError(this, recurq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    rollbackq.exec();
    systemError(this, errmsg, __FILE__, __LINE__);
    return;
  }

  XSqlQuery commitq("COMMIT;");

  done(_todoitemid);
}

void todoItem::sPopulate()
{
  q.prepare( "SELECT * "
             "FROM todoitem "
             "WHERE (todoitem_id=:todoitem_id);" );
  q.bindValue(":todoitem_id", _todoitemid);
  q.exec();
  if (q.first())
  {
    _owner->setUsername(q.value("todoitem_owner_username").toString());
    _assignedTo->setUsername(q.value("todoitem_username").toString());
    _name->setText(q.value("todoitem_name").toString());
    _priority->setNull();
    if(!q.value("todoitem_priority_id").toString().isEmpty())
      _priority->setId(q.value("todoitem_priority_id").toInt());
    _incident->setId(q.value("todoitem_incdt_id").toInt());
    _ophead->setId(q.value("todoitem_ophead_id").toInt());
    _started->setDate(q.value("todoitem_start_date").toDate());
    _assigned->setDate(q.value("todoitem_assigned_date").toDate());
    _due->setDate(q.value("todoitem_due_date").toDate(), true);
    _completed->setDate(q.value("todoitem_completed_date").toDate());
    _description->setText(q.value("todoitem_description").toString());
    _notes->setText(q.value("todoitem_notes").toString());
    _crmacct->setId(q.value("todoitem_crmacct_id").toInt());
    _cntct->setId(q.value("todoitem_cntct_id").toInt());
    _active->setChecked(q.value("todoitem_active").toBool());

    if (q.value("todoitem_status").toString() == "P")
      _pending->setChecked(true);
    else if (q.value("todoitem_status").toString() == "D")
      _deferred->setChecked(true);
    else
      _neither->setChecked(true);

    if (cEdit == _mode && 
	(omfgThis->username()==q.value("todoitem_creator_username").toString() ||
         _privileges->check("MaintainAllToDoItems")))
    {
      _name->setEnabled(true);
      _incident->setEnabled(true);
      _ophead->setEnabled(true);
      _assigned->setEnabled(true);
      _due->setEnabled(true);
      _description->setEnabled(true);
    }

    _alarms->setId(_todoitemid);
    _comments->setId(_todoitemid);
    _documents->setId(_todoitemid);
    _recurring->setParent(q.value("todoitem_recurring_todoitem_id").isNull() ?
                          _todoitemid : q.value("todoitem_recurring_todoitem_id").toInt(),
                          "TODO");
    _cntct->setId(q.value("todoitem_cntct_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void todoItem::sClose()
{
  reject();
}

void todoItem::sHandleIncident()
{
  _crmacct->setEnabled(! _incident->isValid());

  if (_incident->isValid())
  {
    XSqlQuery incdtq;
    incdtq.prepare("SELECT incdt_crmacct_id "
		   "FROM incdt "
		   "WHERE (incdt_id=:incdt_id);");
    incdtq.bindValue(":incdt_id", _incident->id());
    incdtq.exec();
    if (incdtq.first())
      _crmacct->setId(incdtq.value("incdt_crmacct_id").toInt());
    else if (incdtq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, incdtq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
