/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoList.h"
#include "xdialog.h"
#include "todoItem.h"
#include "incident.h"
#include "customer.h"
#include "project.h"
#include "storedProcErrorLookup.h"
#include "task.h"
#include "parameterwidget.h"

#include <QMessageBox>
#include <QSqlError>
#include <QToolBar>

todoList::todoList(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "todoList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("To-Do Items"));
  setReportName("TodoList");
  setMetaSQLOptions("todolist", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Assigned"), "assigned_username", ParameterWidget::User, omfgThis->username());
  parameterWidget()->append(tr("Assigned Pattern"), "assigned_usr_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("Owner Pattern"), "owner_usr_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("CRM Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Start Date Before"), "startStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Start Date After"), "startEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Date Before"), "dueStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Date After"), "dueEndDate", ParameterWidget::Date);

  parameterWidget()->applyDefaultFilterSet();

  connect(_completed, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_todolist, SIGNAL(toggled(bool)), this,   SLOT(sFillList()));
  connect(_incidents, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_projects, SIGNAL(toggled(bool)), this,	SLOT(sFillList()));
  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));

  list()->addColumn(tr("Type"),      _userColumn,  Qt::AlignCenter, true, "type");
  list()->addColumn(tr("Seq"),        _seqColumn,  Qt::AlignRight,  false, "seq");
  list()->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  list()->addColumn(tr("Assigned To"),_userColumn,  Qt::AlignLeft,   true, "usr");
  list()->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  list()->addColumn(tr("Description"),        -1,  Qt::AlignLeft,   true, "descrip");
  list()->addColumn(tr("Status"),  _statusColumn,  Qt::AlignLeft,   true, "status");
  list()->addColumn(tr("Start Date"),_dateColumn,  Qt::AlignLeft,   false, "start");
  list()->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  list()->addColumn(tr("Parent#"),  _orderColumn,  Qt::AlignLeft,   true, "number");
  list()->addColumn(tr("Customer#"),_orderColumn,  Qt::AlignLeft,   false, "cust");
  list()->addColumn(tr("Account#"), _orderColumn,  Qt::AlignLeft,   false, "crmacct_number");
  list()->addColumn(tr("Account Name"),      100,  Qt::AlignLeft,   true, "crmacct_name");
  list()->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");

  newAction()->disconnect();
  QToolButton * newBtn = (QToolButton*)toolBar()->widgetForAction(newAction());
  newBtn->setPopupMode(QToolButton::MenuButtonPopup);
  QAction *menuItem;
  QMenu * todoMenu = new QMenu;
  menuItem = todoMenu->addAction(tr("Incident"), this, SLOT(sNewIncdt()));
  menuItem->setEnabled(_privileges->check("MaintainIncidents"));
  menuItem = todoMenu->addAction(tr("To-Do Item"),   this, SLOT(sNew()));
  menuItem->setShortcut(QKeySequence::New);
  menuItem->setEnabled(_privileges->check("MaintainPersonalTodoList") ||
                       _privileges->check("MaintainOtherTodoLists"));
  newBtn->setMenu(todoMenu);
}

void todoList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->text(3) && _privileges->check("MaintainPersonalTodoList")) ||
      (omfgThis->username() != list()->currentItem()->text(3) && _privileges->check("MaintainOtherTodoLists"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->text(3) && _privileges->check("ViewPersonalTodoList")) ||
      (omfgThis->username() != list()->currentItem()->text(3) && _privileges->check("ViewOtherTodoLists"));

  menuItem = pMenu->addAction(tr("New To-Do..."), this, SLOT(sNew()));
  menuItem->setEnabled(editPriv);

  if (list()->currentItem()->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit To-Do..."), this, SLOT(sEdit()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction(tr("View To-Do..."), this, SLOT(sView()));
    menuItem->setEnabled(viewPriv);

    menuItem = pMenu->addAction(tr("Delete To-Do"), this, SLOT(sDelete()));
    menuItem->setEnabled(editPriv);
  }

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("New Incident..."), this, SLOT(sNewIncdt()));
  menuItem->setEnabled( _privileges->check("MaintainIncidents"));

  if ((list()->altId() == 1 && !list()->currentItem()->text(9).isEmpty()) ||
       list()->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Edit Incident"), this, SLOT(sEditIncident()));
    menuItem->setEnabled(_privileges->check("MaintainIncidents"));
    menuItem = pMenu->addAction(tr("View Incident"), this, SLOT(sViewIncident()));
    menuItem->setEnabled(_privileges->check("ViewIncidents") ||
                         _privileges->check("MaintainIncidents"));
  }
  pMenu->addSeparator();

  if (list()->altId() == 3)
  {
    menuItem = pMenu->addAction(tr("Edit Task"), this, SLOT(sEditTask()));
    menuItem->setEnabled(_privileges->check("MaintainProjects"));
    menuItem = pMenu->addAction(tr("View Task"), this, SLOT(sViewTask()));
    menuItem->setEnabled(_privileges->check("ViewProjects") ||
                         _privileges->check("MaintainProjects"));
    pMenu->addSeparator();
  }

  if (list()->altId() >= 3)
  {
    menuItem = pMenu->addAction(tr("Edit Project"), this, SLOT(sEditProject()));
    menuItem->setEnabled(_privileges->check("MaintainProjects"));
    menuItem = pMenu->addAction(tr("View Project"), this, SLOT(sViewProject()));
    menuItem->setEnabled(_privileges->check("ViewProjects") ||
                         _privileges->check("MaintainProjects"));
  }

  if (!list()->currentItem()->text(10).isEmpty())
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Edit Customer"), this, SLOT(sEditCustomer()));
    menuItem->setEnabled(_privileges->check("MaintainCustomerMasters") || _privileges->check("ViewCustomerMasters"));
    menuItem = pMenu->addAction(tr("View Customer"), this, SLOT(sViewCustomer()));
    menuItem->setEnabled(_privileges->check("ViewCustomerMasters"));
  }
}

enum SetResponse todoList::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;

  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void todoList::sNew()
{
  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");
  //if (_selected->isChecked())
  //  params.append("username", _usr->username());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sNewIncdt()
{
  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");

  incident newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEdit()
{
  if (list()->altId() ==2)
    sEditIncident();
  else if (list()->altId() == 3)
    sEditTask();
  else if (list()->altId() == 4)
    sEditProject();
  else
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("todoitem_id", list()->id());

    todoItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void todoList::sView()
{
  if (list()->altId() ==2)
    sViewIncident();
  else if (list()->altId() == 3)
    sViewTask();
  else if (list()->altId() == 4)
    sViewProject();
  else
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("todoitem_id", list()->id());

    todoItem newdlg(this, "", TRUE);
    newdlg.set(params);

    newdlg.exec();
  }
}

void todoList::sDelete()
{
  QString recurstr;
  QString recurtype;
  if (list()->altId() == 1)
  {
    recurstr = "SELECT MAX(todoitem_due_date) AS max"
               "  FROM todoitem"
               " WHERE todoitem_recurring_todoitem_id=:id"
               "   AND todoitem_id!=:id;" ;
    recurtype = "TODO";
  }
  /* TODO: can't delete incidents from here. why not?
  else if (list()->altId() == 2)
  {
    recurstr = "SELECT MAX(incdt_timestamp) AS max"
               "   FROM incdt"
               " WHERE incdt_recurring_incdt_id=:id;" ;
    recurtype = "INCDT";
  }
   */

  bool deleteAll  = false;
  bool createMore = false;
  if (! recurstr.isEmpty())
  {
    XSqlQuery recurq;
    recurq.prepare(recurstr);
    recurq.bindValue(":id", list()->id());
    recurq.exec();
    if (recurq.first() && !recurq.value("max").isNull())
    {
      QMessageBox askdelete(QMessageBox::Question, tr("Delete Recurring Item?"),
                            tr("<p>This is a recurring item. Do you want to "
                               "delete just this one item or delete all open "
                               "items in this recurrence?"),
                            QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Cancel,
                            this);
      askdelete.setDefaultButton(QMessageBox::Cancel);
      int ret = askdelete.exec();
      if (ret == QMessageBox::Cancel)
        return;
      else if (ret == QMessageBox::YesToAll)
        deleteAll = true;
      // user said delete one but the only one that exists is the base
      else if (ret == QMessageBox::Yes && recurq.value("max").isNull())
        createMore = true;
    }
    else if (recurq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, recurq.lastError().text(), __FILE__, __LINE__);
      return;
    }
    else if (QMessageBox::warning(this, tr("Delete List Item?"),
                                  tr("<p>Are you sure that you want to "
                                     "completely delete the selected item?"),
	  		    QMessageBox::Yes | QMessageBox::No,
			    QMessageBox::No) == QMessageBox::No)
      return;
  }
  else if (QMessageBox::warning(this, tr("Delete List Item?"),
                                tr("<p>Are you sure that you want to "
                                   "completely delete the selected item?"),
	  		    QMessageBox::Yes | QMessageBox::No,
			    QMessageBox::No) == QMessageBox::No)
    return;

  QString procname;
  int procresult = 0;
  if (deleteAll)
  {
    procname = "deleteOpenRecurringItems";
    q.prepare("SELECT deleteOpenRecurringItems(:id, :type, NULL, TRUE)"
              "       AS result;");
    q.bindValue(":id",   list()->id());
    q.bindValue(":type", recurtype);
    q.exec();
    if (q.first())
      procresult = q.value("result").toInt();
  }
  if (procresult >= 0 && createMore)
  {
    procname = "createRecurringItems";
    q.prepare("SELECT createRecurringItems(:id, :type) AS result;");
    q.bindValue(":id",   list()->id());
    q.bindValue(":type", recurtype);
    q.exec();
    if (q.first())
      procresult = q.value("result").toInt();
  }

  // not elseif - error handling for 1 or 2 queries
  if (procresult < 0)
  {
    systemError(this, storedProcErrorLookup(procname, procresult));
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (list()->altId() == 1)
    q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  else if (list()->altId() == 3)
    q.prepare("DELETE FROM prjtask"
              " WHERE (prjtask_id=:todoitem_id); ");
  else if (list()->altId() == 4)
    q.prepare("SELECT deleteProject(:todoitem_id) AS result");
  else
    return;
  q.bindValue(":todoitem_id", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

bool todoList::setParams(ParameterList &params)
{
  if (!_todolist->isChecked() &&
      !_incidents->isChecked() &&
      !_projects->isChecked())
  {
    list()->clear();
    return false;
  }

  if (_todolist->isChecked())
    params.append("todoList");
  if (_completed->isChecked())
    params.append("completed");
  if (_incidents->isChecked())
    params.append("incidents");
  if (_projects->isChecked())
    params.append("projects");

  params.append("todo", tr("To-do"));
  params.append("incident", tr("Incident"));
  params.append("task", tr("Task"));
  params.append("project", tr("Project"));
  params.append("complete", tr("Completed"));
  params.append("deferred", tr("Deferred"));
  params.append("pending", tr("Pending"));
  params.append("inprocess", tr("InProcess"));
  params.append("feedback", tr("Feedback"));
  params.append("confirmed", tr("Confirmed"));
  params.append("assigned", tr("Assigned"));
  params.append("resolved", tr("Resolved"));
  params.append("concept", tr("concept"));
  params.append("new", tr("New"));
  parameterWidget()->appendValue(params);

  return true;
}

int todoList::getIncidentId()
{
  int returnVal = -1;

  if (list()->currentItem()->altId() == 2)
    returnVal = list()->id();
  else if (! list()->currentItem()->text(9).isEmpty())
  {
    XSqlQuery incdt;
    incdt.prepare("SELECT incdt_id FROM incdt WHERE (incdt_number=:number);");
    incdt.bindValue(":number", list()->currentItem()->text(9).toInt());
    if (incdt.exec() && incdt.first())
     returnVal = incdt.value("incdt_id").toInt();
    else if (incdt.lastError().type() != QSqlError::NoError)
      systemError(this, incdt.lastError().databaseText(), __FILE__, __LINE__);
  }

  return returnVal;
}

int todoList::getProjectId()
{
  int returnVal = -1;

  if (list()->currentItem()->altId() == 4)
    returnVal = list()->id();
  else if (list()->currentItem()->altId() == 3)
  {
    XSqlQuery prj;
    prj.prepare("SELECT prjtask_prj_id FROM prjtask WHERE (prjtask_id=:prjtask_id);");
    prj.bindValue(":prjtask_id", list()->id());
    if (prj.exec() && prj.first())
     returnVal = prj.value("prjtask_prj_id").toInt();
    else if (prj.lastError().type() != QSqlError::NoError)
     systemError(this, prj.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (! list()->currentItem()->text(9).isEmpty())
  {
    XSqlQuery prj;
    prj.prepare("SELECT prj_id FROM prj WHERE (prj_number=:number);");
    prj.bindValue(":number", list()->currentItem()->text(9));
    if (prj.exec() && prj.first())
     returnVal = prj.value("prj_id").toInt();
    else if (prj.lastError().type() != QSqlError::NoError)
     systemError(this, prj.lastError().databaseText(), __FILE__, __LINE__);
  }
  return returnVal;
}

void todoList::sEditIncident()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sEditProject()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("prj_id", getProjectId());

  project newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewProject()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prj_id", getProjectId());

  project newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sEditTask()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtask_id", list()->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewTask()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", list()->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sEditCustomer()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", list()->currentItem()->text(10));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());
    params.append("mode","edit");

    customer *newdlg = new customer(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
}

void todoList::sViewCustomer()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", list()->currentItem()->text(10));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());
    params.append("mode","view");

    customer *newdlg = new customer(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
}


