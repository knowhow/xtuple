/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "task.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include "userList.h"

task::task(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_actualExp, SIGNAL(lostFocus()), this, SLOT(sExpensesAdjusted()));
  connect(_budgetExp, SIGNAL(lostFocus()), this, SLOT(sExpensesAdjusted()));
  connect(_actualHours, SIGNAL(lostFocus()), this, SLOT(sHoursAdjusted()));
  connect(_budgetHours, SIGNAL(lostFocus()), this, SLOT(sHoursAdjusted()));
  
  _budgetHours->setValidator(omfgThis->qtyVal());
  _actualHours->setValidator(omfgThis->qtyVal());
  _budgetExp->setValidator(omfgThis->costVal());
  _actualExp->setValidator(omfgThis->costVal());
  _balanceHours->setPrecision(omfgThis->qtyVal());
  _balanceExp->setPrecision(omfgThis->costVal());

  _prjid = -1;
  _prjtaskid = -1;
  
  _owner->setType(UsernameLineEdit::UsersActive);
  _assignedTo->setType(UsernameLineEdit::UsersActive);
}

task::~task()
{
  // no need to delete child widgets, Qt does it all for us
}

void task::languageChange()
{
  retranslateUi(this);
}

enum SetResponse task::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
    _prjid = param.toInt();

  param = pParams.value("prj_owner_username", &valid);
  if (valid)
    _owner->setUsername(param.toString());

  param = pParams.value("prj_username", &valid);
  if (valid)
    _assignedTo->setUsername(param.toString());

  param = pParams.value("prj_start_date", &valid);
  if (valid)
    _started->setDate(param.toDate());

  param = pParams.value("prj_assigned_date", &valid);
  if (valid)
    _assigned->setDate(param.toDate());

  param = pParams.value("prj_due_date", &valid);
  if (valid)
    _due->setDate(param.toDate());

  param = pParams.value("prj_completed_date", &valid);
  if (valid)
    _completed->setDate(param.toDate());

  param = pParams.value("prjtask_id", &valid);
  if (valid)
  {
    _prjtaskid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('prjtask_prjtask_id_seq') AS prjtask_id;");
      if (q.first())
        _prjtaskid = q.value("prjtask_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
      }

      _alarms->setId(_prjtaskid);
      _comments->setId(_prjtaskid);
    }
    if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(false);
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _status->setEnabled(false);
      _budgetHours->setEnabled(false);
      _actualHours->setEnabled(false);
      _budgetExp->setEnabled(false);
      _actualExp->setEnabled(false);
      _owner->setEnabled(false);
      _assignedTo->setEnabled(false);
      _due->setEnabled(false);
      _assigned->setEnabled(false);
      _started->setEnabled(false);
      _completed->setEnabled(false);
      _alarms->setEnabled(false);
      _comments->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void task::populate()
{
  q.prepare( "SELECT prjtask.* "
             "FROM prjtask "
             "WHERE (prjtask_id=:prjtask_id);" );
  q.bindValue(":prjtask_id", _prjtaskid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("prjtask_number"));
    _name->setText(q.value("prjtask_name"));
    _descrip->setText(q.value("prjtask_descrip").toString());
    _owner->setUsername(q.value("prjtask_owner_username").toString());
    _assignedTo->setUsername(q.value("prjtask_username").toString());
    _started->setDate(q.value("prjtask_start_date").toDate());
    _assigned->setDate(q.value("prjtask_assigned_date").toDate());
    _due->setDate(q.value("prjtask_due_date").toDate());
    _completed->setDate(q.value("prjtask_completed_date").toDate());

    QString status = q.value("prjtask_status").toString();
    if("P" == status)
      _status->setCurrentIndex(0);
    else if("O" == status)
      _status->setCurrentIndex(1);
    else if("C" == status)
      _status->setCurrentIndex(2);

    _budgetHours->setText(formatQty(q.value("prjtask_hours_budget").toDouble()));
    _actualHours->setText(formatQty(q.value("prjtask_hours_actual").toDouble()));
    _budgetExp->setText(formatCost(q.value("prjtask_exp_budget").toDouble()));
    _actualExp->setText(formatCost(q.value("prjtask_exp_actual").toDouble()));

    _alarms->setId(_prjtaskid);
    _comments->setId(_prjtaskid);    
    sHoursAdjusted();
    sExpensesAdjusted();

    //if (q.value("prjtask_anyuser").toBool())
    //  _anyUser->setChecked(TRUE);
    //else
    //  _userList->setChecked(TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  //sFillUserList();
}

void task::sSave()
{
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Project Task"),
                            tr("You must enter a valid Number.") );
      _number->setFocus();
      return;
  }
  
  if (_name->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Project Task"),
                            tr("You must enter a valid Name.") );
      _name->setFocus();
      return;
  }

  if (!_due->isValid())
  {
      QMessageBox::warning( this, tr("Cannot Save Project Task"),
                            tr("You must enter a valid due date.") );
      _due->setFocus();
      return;
  }
  
  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO prjtask "
               "( prjtask_id, prjtask_prj_id, prjtask_number,"
               "  prjtask_name, prjtask_descrip, prjtask_status,"
               "  prjtask_hours_budget, prjtask_hours_actual,"
               "  prjtask_exp_budget, prjtask_exp_actual,"
               "  prjtask_start_date, prjtask_due_date,"
               "  prjtask_assigned_date, prjtask_completed_date,"
               "  prjtask_owner_username, prjtask_username ) "
               "VALUES "
               "( :prjtask_id, :prjtask_prj_id, :prjtask_number,"
               "  :prjtask_name, :prjtask_descrip, :prjtask_status,"
               "  :prjtask_hours_budget, :prjtask_hours_actual,"
               "  :prjtask_exp_budget, :prjtask_exp_actual,"
               "  :prjtask_start_date, :prjtask_due_date,"
               "  :prjtask_assigned_date, :prjtask_completed_date,"
               "  :prjtask_owner_username, :username );" );
    q.bindValue(":prjtask_prj_id", _prjid);
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE prjtask "
               "SET prjtask_number=:prjtask_number, prjtask_name=:prjtask_name,"
               "    prjtask_descrip=:prjtask_descrip, prjtask_status=:prjtask_status,"
               "    prjtask_hours_budget=:prjtask_hours_budget,"
               "    prjtask_hours_actual=:prjtask_hours_actual,"
               "    prjtask_exp_budget=:prjtask_exp_budget,"
               "    prjtask_exp_actual=:prjtask_exp_actual,"
               "    prjtask_owner_username=:prjtask_owner_username,"
               "    prjtask_username=:username,"
               "    prjtask_start_date=:prjtask_start_date,"
               "    prjtask_due_date=:prjtask_due_date,"
               "    prjtask_assigned_date=:prjtask_assigned_date,"
               "    prjtask_completed_date=:prjtask_completed_date "
               "WHERE (prjtask_id=:prjtask_id);" );

  q.bindValue(":prjtask_id", _prjtaskid);
  q.bindValue(":prjtask_number", _number->text());
  q.bindValue(":prjtask_name", _name->text());
  q.bindValue(":prjtask_descrip", _descrip->toPlainText());
  q.bindValue(":prjtask_owner_username", _owner->username());
  q.bindValue(":username",   _assignedTo->username());
  q.bindValue(":prjtask_start_date",	_started->date());
  q.bindValue(":prjtask_due_date",	_due->date());
  q.bindValue(":prjtask_assigned_date",	_assigned->date());
  q.bindValue(":prjtask_completed_date",	_completed->date());
  //q.bindValue(":prjtask_anyuser", QVariant(_anyUser->isChecked()));
  q.bindValue(":prjtask_hours_budget", _budgetHours->text().toDouble());
  q.bindValue(":prjtask_hours_actual", _actualHours->text().toDouble());
  q.bindValue(":prjtask_exp_budget", _budgetExp->text().toDouble());
  q.bindValue(":prjtask_exp_actual", _actualExp->text().toDouble());

  switch(_status->currentIndex())
  {
    case 0:
    default:
      q.bindValue(":prjtask_status", "P");
      break;
    case 1:
      q.bindValue(":prjtask_status", "O");
      break;
    case 2:
      q.bindValue(":prjtask_status", "C");
      break;
  }

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_prjtaskid);
}

void task::sHoursAdjusted()
{
  _balanceHours->setText(formatQty(_budgetHours->text().toDouble() - _actualHours->text().toDouble()));
}

void task::sExpensesAdjusted()
{
  _balanceExp->setText(formatCost(_budgetExp->text().toDouble() - _actualExp->text().toDouble()));
}

void task::sNewUser()
{
/*
  userList newdlg(this, "", TRUE);
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
  {
    QString username = newdlg.username();
    q.prepare( "SELECT prjtaskuser_id "
               "FROM prjtaskuser "
               "WHERE ( (prjtaskuser_username=:username)"
               " AND (prjtaskuser_prjtask_id=:prjtask_id) );" );
    q.bindValue(":username", username);
    q.bindValue(":prjtask_id", _prjtaskid);
    q.exec();
    if (!q.first())
    {
      q.prepare( "INSERT INTO prjtaskuser "
                 "( prjtaskuser_prjtask_id, prjtaskuser_username ) "
                 "VALUES "
                 "( :prjtaskuser_prjtask_id, :prjtaskuser_username );" );
      q.bindValue(":prjtaskuser_username", username);
      q.bindValue(":prjtaskuser_prjtask_id", _prjtaskid);
      q.exec();
      sFillUserList();
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
*/
}

void task::sDeleteUser()
{
/*
  q.prepare( "DELETE FROM prjtaskuser "
             "WHERE ( (prjtaskuser_username=:username)"
             " AND (prjtaskuser_prjtask_id=:prjtask_id) );" );
  q.bindValue(":username", _usr->username());
  q.bindValue(":prjtask_id", _prjtaskid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillUserList();
*/
}


void task::sFillUserList()
{
/*
  q.prepare( "SELECT prjtaskuser_id, usr_username, usr_propername "
             "FROM prjtaskuser, usr "
             "WHERE ( (prjtaskuser_username=usr_username)"
             " AND (prjtaskuser_prjtask_id=:prjtask_id) );" );
  q.bindValue(":prjtask_id", _prjtaskid);
  q.exec();
  _usr->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
*/
}

