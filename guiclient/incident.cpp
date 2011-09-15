/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incident.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "arOpenItem.h"
#include "returnAuthorization.h"
#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "characteristicAssignment.h"
#include <openreports.h>

incident::incident(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  _statusCodes << "N" << "F" << "C" << "A" << "R" << "L";
  setupUi(this);

  _incdtid = -1;
  _saved = false;
  _aropenid = -1;

  _print = _buttonBox->addButton(tr("Print"),QDialogButtonBox::ActionRole);
  _print->setObjectName("_print");
  _buttonBox->button(QDialogButtonBox::Save)->setObjectName("_save");

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  connect(_assignedTo,    SIGNAL(newId(int)),       this, SLOT(sAssigned()));
  connect(_buttonBox,     SIGNAL(rejected()),        this,       SLOT(sCancel()));
  connect(_crmacct,       SIGNAL(newId(int)),       this,       SLOT(sCRMAcctChanged(int)));
  connect(_deleteTodoItem, SIGNAL(clicked()),       this,       SLOT(sDeleteTodoItem()));
  connect(_editTodoItem,  SIGNAL(clicked()),        this,       SLOT(sEditTodoItem()));
  connect(_item,          SIGNAL(newId(int)),     _lotserial,   SLOT(setItemId(int)));
  connect(_newTodoItem,   SIGNAL(clicked()),        this,       SLOT(sNewTodoItem()));
  //connect(_return,      SIGNAL(clicked()),        this, SLOT(sReturn()));
  connect(_buttonBox,     SIGNAL(accepted()),        this,       SLOT(sSave()));
  connect(_print,         SIGNAL(clicked()),        this,       SLOT(sPrint()));
  connect(_todoList,      SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  connect(_todoList,      SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this,         SLOT(sPopulateTodoMenu(QMenu*)));
  connect(_todoList,      SIGNAL(valid(bool)),      this, SLOT(sHandleTodoPrivs()));
  connect(_viewAR,        SIGNAL(clicked()),        this, SLOT(sViewAR()));
  connect(_viewTodoItem,  SIGNAL(clicked()),        this,       SLOT(sViewTodoItem()));
  connect(_deleteCharacteristic,SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
  connect(_editCharacteristic,	SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_newCharacteristic,	SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
  connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
  connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));

  _severity->setType(XComboBox::IncidentSeverity);
  _priority->setType(XComboBox::IncidentPriority);
  _resolution->setType(XComboBox::IncidentResolution);
  _category->setType(XComboBox::IncidentCategory);
  _lotserial->setStrict(false);

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");

  _incdthist->addColumn(tr("Username"),     _userColumn, Qt::AlignLeft, true, "incdthist_username");
  _incdthist->addColumn(tr("Date/Time"),_timeDateColumn, Qt::AlignLeft, true, "incdthist_timestamp");
  _incdthist->addColumn(tr("Description"),           -1, Qt::AlignLeft, true, "incdthist_descrip");

  _todoList->addColumn(tr("Priority"),      _userColumn, Qt::AlignRight, true, "incdtpriority_name");
  _todoList->addColumn(tr("User"),          _userColumn, Qt::AlignLeft,  true, "todoitem_username");
  _todoList->addColumn(tr("Name"),                  100, Qt::AlignLeft,  true, "todoitem_name");
  _todoList->addColumn(tr("Description"),            -1, Qt::AlignLeft,  true, "todoitem_description");
  _todoList->addColumn(tr("Status"),      _statusColumn, Qt::AlignLeft,  true, "todoitem_status");
  _todoList->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignLeft,  true, "todoitem_due_date");

  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);
  _assignedTo->setType(UsernameLineEdit::UsersActive);

  if (_metrics->boolean("LotSerialControl"))
  {
    connect(_item, SIGNAL(valid(bool)), _lotserial, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(newId(int)),  _lotserial, SLOT(setItemId(int)));
  }
  else
    _lotserial->setVisible(false);

  if(!_metrics->boolean("IncidentsPublicPrivate"))
    _public->hide();
  _public->setChecked(_metrics->boolean("IncidentPublicDefault"));

  // because this causes a pop-behind situation we are hiding for now.
  //_return->hide();
}

incident::~incident()
{
  // no need to delete child widgets, Qt does it all for us
}

void incident::languageChange()
{
  retranslateUi(this);
}

enum SetResponse incident::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdt_id", &valid);
  if (valid)
  {
    _incdtid = param.toInt();
    populate();
    _lotserial->setItemId(_item->id());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.exec("SELECT nextval('incdt_incdt_id_seq') AS incdt_id, "
             "fetchIncidentNumber() AS number;");
      if(q.first())
      {
        _incdtid=q.value("incdt_id").toInt();
        _number->setText(q.value("number").toString());
        _comments->setId(_incdtid);
        _documents->setId(_incdtid);
        _alarms->setId(_incdtid);
        _recurring->setParent(_incdtid, "INCDT");
        _print->hide();
      }
      else
      {
        QMessageBox::critical( omfgThis, tr("Database Error"),
                               tr( "A Database Error occured in incident::New:"
                                   "\n%1" ).arg(q.lastError().text()));
        reject();
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _crmacct->setEnabled(true);
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _crmacct->setEnabled(false);
      _cntct->setEnabled(false);
      _assignedTo->setEnabled(false);
      _category->setEnabled(false);
      _status->setEnabled(false);
      _resolution->setEnabled(false);
      _severity->setEnabled(false);
      _priority->setEnabled(false);
      _item->setReadOnly(true);
      _lotserial->setEnabled(false);
      _description->setEnabled(false);
      _notes->setEnabled(false);
      _deleteTodoItem->setEnabled(false);
      _editTodoItem->setEnabled(false);
      _newTodoItem->setEnabled(false);
      disconnect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      disconnect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      disconnect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
      _newCharacteristic->setEnabled(false);
      _owner->setEnabled(false);

      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
      _buttonBox->setFocus();
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      _alarms->setReadOnly(true);
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _cntct->setId(param.toInt());
  }

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    q.prepare("SELECT aropen_doctype, aropen_docnumber, "
              "       CASE WHEN (aropen_doctype='C') THEN :creditMemo"
              "            WHEN (aropen_doctype='D') THEN :debitMemo"
              "            WHEN (aropen_doctype='I') THEN :invoice"
              "            WHEN (aropen_doctype='R') THEN :cashdeposit"
              "            ELSE '' END AS docType "
              "FROM aropen "
              "WHERE (aropen_id=:aropen_id);");
    q.bindValue(":aropen_id", _aropenid);
    q.bindValue(":creditMemo", tr("Credit Memo"));
    q.bindValue(":debitMemo", tr("Debit Memo"));
    q.bindValue(":invoice", tr("Invoice"));
    q.bindValue(":cashdeposit", tr("Customer Deposit"));
    q.exec();
    if (q.first())
    {
      if (_metrics->value("DefaultARIncidentStatus").toInt())
        _category->setId(_metrics->value("DefaultARIncidentStatus").toInt());
      _ardoctype=q.value("aropen_doctype").toString();
      _docType->setText(q.value("docType").toString());
      _docNumber->setText(q.value("aropen_docnumber").toString());
      _description->setText(QString("%1 #%2").arg(q.value("docType").toString()).arg(q.value("aropen_docnumber").toString()));
    }
  }

  sHandleTodoPrivs();
  return NoError;
}

int incident::id() const
{
  return _incdtid;
}

int incident::mode() const
{
  return _mode;
}

int incident::aropenid() const
{
  return _aropenid;
}

QString incident::arDoctype() const
{
  return _ardoctype;
}

void incident::sCancel()
{
  if (cNew == _mode)
  {
    q.prepare("SELECT deleteIncident(:incdt_id) AS result;");
    q.bindValue(":incdt_id", _incdtid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteIncident", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void incident::sSave()
{
  if (! save(false)) // if error
    return;
  {
   done(_incdtid);
  }
}

bool incident::save(bool partial)
{
  if (! partial)
  {
    if(_crmacct->id() == -1)
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
        tr("You must specify the Account that this incident is for.") );
      return false;
    }

    if(_cntct->id() <= 0 && _cntct->name().simplified().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
        tr("You must specify a Contact for this Incident.") );
      return false;
    }

    if(_description->text().trimmed().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
        tr("You must specify a description for this incident report.") );
      _description->setFocus();
      return false;
    }

    if (_status->currentIndex() == 3 && _assignedTo->username().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
        tr("You must specify an assignee when the status is assigned.") );
      _description->setFocus();
      return false;
    }
  }

  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return false;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (!q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (cNew == _mode && !_saved)
    q.prepare("INSERT INTO incdt"
              "      (incdt_id, incdt_number, incdt_crmacct_id, incdt_cntct_id,"
              "       incdt_summary, incdt_descrip, incdt_item_id,"
              "       incdt_status, incdt_assigned_username,"
              "       incdt_incdtcat_id, incdt_incdtseverity_id,"
              "       incdt_incdtpriority_id, incdt_incdtresolution_id,"
              "       incdt_ls_id, incdt_aropen_id, incdt_owner_username,"
              "       incdt_prj_id, incdt_public,"
              "       incdt_recurring_incdt_id) "
              "VALUES(:incdt_id, :incdt_number, :incdt_crmacct_id, :incdt_cntct_id,"
              "       :incdt_description, :incdt_notes, :incdt_item_id,"
              "       :incdt_status, :incdt_assigned_username,"
              "       :incdt_incdtcat_id, :incdt_incdtseverity_id,"
              "       :incdt_incdtpriority_id, :incdt_incdtresolution_id,"
              "       :incdt_ls_id, :incdt_aropen_id, :incdt_owner_username,"
              "       :incdt_prj_id, :incdt_public,"
              "       :incdt_recurring_incdt_id);" );
  else if (cEdit == _mode || _saved)
    q.prepare("UPDATE incdt"
              "   SET incdt_cntct_id=:incdt_cntct_id,"
              "       incdt_crmacct_id=:incdt_crmacct_id,"
              "       incdt_summary=:incdt_description,"
              "       incdt_descrip=:incdt_notes,"
              "       incdt_item_id=:incdt_item_id,"
              "       incdt_status=:incdt_status,"
              "       incdt_assigned_username=:incdt_assigned_username,"
              "       incdt_incdtcat_id=:incdt_incdtcat_id,"
              "       incdt_incdtpriority_id=:incdt_incdtpriority_id,"
              "       incdt_incdtseverity_id=:incdt_incdtseverity_id,"
              "       incdt_incdtresolution_id=:incdt_incdtresolution_id,"
              "       incdt_ls_id=:incdt_ls_id,"
              "       incdt_owner_username=:incdt_owner_username,"
              "       incdt_prj_id=:incdt_prj_id,"
              "       incdt_public=:incdt_public,"
              "       incdt_recurring_incdt_id=:incdt_recurring_incdt_id"
              " WHERE (incdt_id=:incdt_id); ");

  q.bindValue(":incdt_id", _incdtid);
  q.bindValue(":incdt_number", _number->text());
  q.bindValue(":incdt_owner_username", _owner->username());
  if (_crmacct->id() > 0)
    q.bindValue(":incdt_crmacct_id", _crmacct->id());
  if (_cntct->id() > 0)
    q.bindValue(":incdt_cntct_id", _cntct->id());
  q.bindValue(":incdt_description", _description->text().trimmed());
  q.bindValue(":incdt_notes", _notes->toPlainText().trimmed());
  if(-1 != _item->id())
    q.bindValue(":incdt_item_id", _item->id());
  q.bindValue(":incdt_assigned_username", _assignedTo->username());
  q.bindValue(":incdt_status", _statusCodes.at(_status->currentIndex()));
  if(_category->isValid())
    q.bindValue(":incdt_incdtcat_id", _category->id());
  if(_severity->isValid())
    q.bindValue(":incdt_incdtseverity_id", _severity->id());
  if(_priority->isValid())
    q.bindValue(":incdt_incdtpriority_id", _priority->id());
  if(_resolution->isValid())
    q.bindValue(":incdt_incdtresolution_id", _resolution->id());
  if ((_item->id() != -1) && (_lotserial->id() != -1))
    q.bindValue(":incdt_ls_id", _lotserial->id());
  if (_aropenid > 0)
    q.bindValue(":incdt_aropen_id", _aropenid);
  if (_recurring->isRecurring())
    q.bindValue(":incdt_recurring_incdt_id", _recurring->parentId());
  if (_project->id() > 0)
    q.bindValue(":incdt_prj_id", _project->id());
  q.bindValue(":incdt_public", _public->isChecked());

  if(!q.exec() && q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    rollback.exec();
    systemError(this, errmsg, __FILE__, __LINE__);
    return false;
  }

  q.exec("COMMIT;");
  if(q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  _saved = true;
  return true;
}

void incident::sFillHistoryList()
{
  q.prepare("SELECT * "
            "  FROM incdthist"
            " WHERE (incdthist_incdt_id=:incdt_id)"
            " ORDER BY incdthist_timestamp; ");
  q.bindValue(":incdt_id", _incdtid);
  q.bindValue(":new", tr("New Incident"));
  q.bindValue(":status", tr("Status"));
  q.bindValue(":category", tr("Category"));
  q.bindValue(":severity", tr("Severity"));
  q.bindValue(":priority", tr("Priority"));
  q.bindValue(":resolution", tr("Resolution"));
  q.bindValue(":assignedto", tr("Assigned To"));
  q.bindValue(":notes", tr("Comment"));
  q.bindValue(":contact", tr("Contact"));
  q.exec();
  _incdthist->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incident::populate()
{
  q.prepare("SELECT incdt_number,"
            "       incdt_crmacct_id,"
            "       COALESCE(incdt_cntct_id,-1) AS incdt_cntct_id,"
            "       (cntct_first_name || ' ' || cntct_last_name) AS cntct_name,"
            "       incdt_summary, incdt_descrip,"
            "       incdt_item_id, incdt_ls_id,"
            "       incdt_status, incdt_assigned_username,"
            "       incdt_incdtcat_id, incdt_incdtseverity_id,"
            "       incdt_incdtpriority_id, incdt_incdtresolution_id,"
            "       incdt_owner_username, incdt_recurring_incdt_id,"
            "       COALESCE(incdt_aropen_id, -1) AS docId,"
            "       COALESCE(aropen_docnumber, '') AS docNumber,"
            "       CASE WHEN (aropen_doctype='C') THEN :creditMemo"
            "            WHEN (aropen_doctype='D') THEN :debitMemo"
            "            WHEN (aropen_doctype='I') THEN :invoice"
            "            WHEN (aropen_doctype='R') THEN :cashdeposit"
            "            ELSE ''"
            "       END AS docType, "
            "       COALESCE(incdt_prj_id,-1) AS incdt_prj_id,"
            "       COALESCE(incdt_public, false) AS incdt_public,"
            "       aropen_doctype "
            "FROM incdt LEFT OUTER JOIN cntct ON (incdt_cntct_id=cntct_id)"
            "           LEFT OUTER JOIN aropen ON (incdt_aropen_id=aropen_id) "
            "WHERE (incdt_id=:incdt_id); ");
  q.bindValue(":incdt_id", _incdtid);
  q.bindValue(":creditMemo", tr("Credit Memo"));
  q.bindValue(":debitMemo", tr("Debit Memo"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":cashdeposit", tr("Customer Deposit"));
  q.exec();
  if(q.first())
  {
    _cntct->setId(q.value("incdt_cntct_id").toInt());
    _crmacct->setId(q.value("incdt_crmacct_id").toInt());
    _owner->setUsername(q.value("incdt_owner_username").toString());
    _number->setText(q.value("incdt_number").toString());
    _assignedTo->setUsername(q.value("incdt_assigned_username").toString());
    _category->setNull();
    if(!q.value("incdt_incdtcat_id").toString().isEmpty())
      _category->setId(q.value("incdt_incdtcat_id").toInt());
    _status->setCurrentIndex(_statusCodes.indexOf(q.value("incdt_status").toString()));
    _severity->setNull();
    if(!q.value("incdt_incdtseverity_id").toString().isEmpty())
      _severity->setId(q.value("incdt_incdtseverity_id").toInt());
    _priority->setNull();
    if(!q.value("incdt_incdtpriority_id").toString().isEmpty())
      _priority->setId(q.value("incdt_incdtpriority_id").toInt());
    _resolution->setNull();
    if(!q.value("incdt_incdtresolution_id").toString().isEmpty())
      _resolution->setId(q.value("incdt_incdtresolution_id").toInt());
    if(!q.value("incdt_item_id").toString().isEmpty())
      _item->setId(q.value("incdt_item_id").toInt());
    else
      _item->setId(-1);
    if(!q.value("incdt_ls_id").toString().isEmpty())
      _lotserial->setId(q.value("incdt_ls_id").toInt());
    else
      _lotserial->setId(-1);
    _description->setText(q.value("incdt_summary").toString());
    _notes->setText(q.value("incdt_descrip").toString());

    _comments->setId(_incdtid);
    _documents->setId(_incdtid);
    _alarms->setId(_incdtid);

    _project->setId(q.value("incdt_prj_id").toInt());
    _public->setChecked(q.value("incdt_public").toBool());
        
    _docType->setText(q.value("docType").toString());
    _docNumber->setText(q.value("docNumber").toString());
    _aropenid = q.value("docId").toInt();
    _ardoctype = q.value("aropen_doctype").toString();
    if (_aropenid > 0)
      _viewAR->setEnabled(true);

    _recurring->setParent(q.value("incdt_recurring_incdt_id").isNull() ?
                          _incdtid : q.value("incdt_recurring_incdt_id").toInt(),
                          "INCDT");

    sFillCharacteristicsList();
    sFillHistoryList();
    sFillTodoList();

    emit populated();
  }
}

void incident::sCRMAcctChanged(const int newid)
{
  _cntct->setSearchAcct(newid);
}

void incident::sNewTodoItem()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("incdt_id", _incdtid);
  params.append("priority_id", _priority->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTodoList();
}

void incident::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTodoList();
}

void incident::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incident::sDeleteTodoItem()
{
  q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  q.bindValue(":todoitem_id", _todoList->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
    else
      sFillTodoList();
    }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incident::sFillTodoList()
{
  q.prepare("SELECT todoitem_id, *, "
            "       firstLine(todoitem_notes) AS todoitem_notes, "
            "       CASE WHEN (todoitem_status != 'C' AND"
            "                  todoitem_due_date < CURRENT_DATE) THEN 'expired'"
            "            WHEN (todoitem_status != 'C' AND"
            "                  todoitem_due_date > CURRENT_DATE) THEN 'future'"
            "       END AS todoitem_due_date_qtforegroundrole "
            "  FROM todoitem "
            "       LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
            "WHERE ( (todoitem_incdt_id=:incdt_id) "
            "  AND   (todoitem_active) ) "
            "ORDER BY todoitem_due_date, todoitem_username;");

  q.bindValue(":incdt_id", _incdtid);
  q.exec();
  _todoList->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incident::sPopulateTodoMenu(QMenu *pMenu)
{
  QAction *menuItem;

  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainOtherTodoLists")) );

  bool viewPriv =
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewOtherTodoLists"));

  menuItem = pMenu->addAction(tr("New..."), this, SLOT(sNewTodoItem()));
  menuItem->setEnabled(newPriv);

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditTodoItem()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewTodoItem()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDeleteTodoItem()));
  menuItem->setEnabled(editPriv);
}

void incident::sHandleTodoPrivs()
{
  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = false;
  bool viewPriv = false;

  if(_todoList->currentItem())
  {
    editPriv = (cNew == _mode || cEdit == _mode) && (
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainOtherTodoLists")) );

    viewPriv =
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewOtherTodoLists"));
  }

  _newTodoItem->setEnabled(newPriv);
  _editTodoItem->setEnabled(editPriv && _todoList->id() > 0);
  _viewTodoItem->setEnabled((editPriv || viewPriv) && _todoList->id() > 0);
  _deleteTodoItem->setEnabled(editPriv && _todoList->id() > 0);

  if (editPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_viewTodoItem, SLOT(animateClick()));
    connect(_todoList,  SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_editTodoItem, SLOT(animateClick()));
    connect(_todoList,  SIGNAL(itemSelected(int)), _viewTodoItem, SLOT(animateClick()));
  }
}

void incident::sReturn()
{
  if (! save(true))
    return;

  ParameterList params;
  q.prepare("SELECT rahead_id FROM rahead WHERE rahead_incdt_id=:incdt_id");
  q.bindValue(":incdt_id", _incdtid);
  q.exec();
  if(q.first())
  {
    params.append("mode", "edit");
    params.append("rahead_id", q.value("rahead_id").toInt());
  }
  else
  {
    params.append("mode", "new");
    params.append("incdt_id", _incdtid);
  }

  returnAuthorization * newdlg = new returnAuthorization(this);
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
                          tr("The new Return Authorization could not be created"));
}

void incident::sViewAR()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropenid);

  arOpenItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incident::sPrint()
{
  if (_incdtid != -1)
  {
    ParameterList params;
    params.append("incdt_id", _incdtid);
    params.append("print");

    orReport report("Incident", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
//  ToDo
}

void incident::sAssigned()
{
  if (_status->currentIndex() < 3 && !_assignedTo->username().isEmpty())
    _status->setCurrentIndex(3);
}

void incident::sNewCharacteristic()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("incdt_id", _incdtid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristicsList();
}

void incident::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristicsList();
}

void incident::sDeleteCharacteristic()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sFillCharacteristicsList();
}

void incident::sFillCharacteristicsList()
{
  XSqlQuery qry;
  qry.prepare( "SELECT charass_id, char_name, "
               " CASE WHEN char_type < 2 THEN "
               "   charass_value "
               " ELSE "
               "   formatDate(charass_value::date) "
               "END AS charass_value "
               "FROM charass, char "
               "WHERE ( (charass_target_type='INCDT')"
               " AND (charass_char_id=char_id)"
               " AND (charass_target_id=:incdt_id) ) "
               "ORDER BY char_order, char_name;" );
  qry.bindValue(":incdt_id", _incdtid);
  qry.exec();
  _charass->populate(qry);
}
