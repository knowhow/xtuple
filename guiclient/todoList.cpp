/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
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
#include "opportunity.h"
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

  parameterWidget()->append(tr("User"), "username", ParameterWidget::User, omfgThis->username());
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("Assigned To"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("CRM Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Start Date on or Before"), "startStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Start Date on or After"), "startEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Date on or Before"), "dueStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Date on or After"), "dueEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Show Completed"), "completed", ParameterWidget::Exists);
  parameterWidget()->append(tr("Show Completed Only"), "completedonly", ParameterWidget::Exists);

  parameterWidget()->applyDefaultFilterSet();

  connect(_opportunities, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_todolist, SIGNAL(toggled(bool)), this,   SLOT(sFillList()));
  connect(_incidents, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_projects, SIGNAL(toggled(bool)), this,	SLOT(sFillList()));
  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  list()->addColumn(tr("Type"),      _userColumn,  Qt::AlignCenter, true, "type");
  list()->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  list()->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");
  list()->addColumn(tr("Assigned To"),_userColumn, Qt::AlignLeft,   true, "assigned");
  list()->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  list()->addColumn(tr("Notes"),        -1,  Qt::AlignLeft,   true, "notes");
  list()->addColumn(tr("Stage"),   _statusColumn,  Qt::AlignLeft,   true, "stage");
  list()->addColumn(tr("Start Date"),_dateColumn,  Qt::AlignLeft,   false, "start");
  list()->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  list()->addColumn(tr("Account#"), _orderColumn,  Qt::AlignLeft,   false, "crmacct_number");
  list()->addColumn(tr("Account Name"),      100,  Qt::AlignLeft,   true, "crmacct_name");
  list()->addColumn(tr("Parent"),            100,  Qt::AlignLeft,   false, "parent");
  list()->addColumn(tr("Customer"),    _ynColumn,  Qt::AlignLeft,   false, "cust");

  QToolButton * newBtn = (QToolButton*)toolBar()->widgetForAction(newAction());
  newBtn->setPopupMode(QToolButton::MenuButtonPopup);
  QAction *menuItem;
  QMenu * todoMenu = new QMenu;
  menuItem = todoMenu->addAction(tr("To-Do Item"),   this, SLOT(sNew()));
  menuItem->setShortcut(QKeySequence::New);
  menuItem->setEnabled(_privileges->check("MaintainPersonalToDoItems") ||
                       _privileges->check("MaintainAllToDoItems"));
  menuItem = todoMenu->addAction(tr("Opportunity"), this, SLOT(sNewOpportunity()));
  menuItem->setEnabled(_privileges->check("MaintainPersonalOpportunities") ||
                       _privileges->check("MaintainAllOpportunities"));
  menuItem = todoMenu->addAction(tr("Incident"), this, SLOT(sNewIncdt()));
  menuItem->setEnabled(_privileges->check("MaintainPersonalIncidents") ||
                       _privileges->check("MaintainAllIncidents"));
  menuItem = todoMenu->addAction(tr("Project"), this, SLOT(sNewProject()));
  menuItem->setEnabled(_privileges->check("MaintainPersonalProjects") ||
                       _privileges->check("MaintainAllProjects"));
  newBtn->setMenu(todoMenu);
}

void todoList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv = false;
  bool viewPriv = false;

  if (list()->currentItem()->altId() == 1)
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalToDoItems")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalToDoItems")) ||
        (_privileges->check("MaintainAllToDoItems"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalToDoItems")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalToDoItems")) ||
        (_privileges->check("ViewAllToDoItems"));

    menuItem = pMenu->addAction(tr("Edit To-Do..."), this, SLOT(sEdit()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction(tr("View To-Do..."), this, SLOT(sView()));
    menuItem->setEnabled(viewPriv);

    menuItem = pMenu->addAction(tr("Delete To-Do"), this, SLOT(sDelete()));
    menuItem->setEnabled(editPriv);
  }

  pMenu->addSeparator();

  if (list()->altId() == 2 ||
      (list()->currentItem()->altId() == 1 &&
       list()->currentItem()->rawValue("parent") == "INCDT"))
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalIncidents")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalIncidents")) ||
        (_privileges->check("MaintainAllIncidents"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalIncidents")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalIncidents")) ||
        (_privileges->check("ViewAllIncidents"));

    menuItem = pMenu->addAction(tr("Edit Incident"), this, SLOT(sEditIncident()));
    menuItem->setEnabled(editPriv);
    menuItem = pMenu->addAction(tr("View Incident"), this, SLOT(sViewIncident()));
    menuItem->setEnabled(viewPriv);
  }
  pMenu->addSeparator();

  if (list()->altId() == 3)
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalProjects")) ||
        (_privileges->check("MaintainAllProjects"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalProjects")) ||
        (_privileges->check("ViewAllProjects"));

    menuItem = pMenu->addAction(tr("Edit Task"), this, SLOT(sEditTask()));
    menuItem->setEnabled(editPriv);
    menuItem = pMenu->addAction(tr("View Task"), this, SLOT(sViewTask()));
    menuItem->setEnabled(viewPriv);
    pMenu->addSeparator();
  }

  if (list()->altId() == 3 || list()->altId() == 4)
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalProjects")) ||
        (_privileges->check("MaintainAllProjects"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalProjects")) ||
        (_privileges->check("ViewAllProjects"));

    menuItem = pMenu->addAction(tr("Edit Project"), this, SLOT(sEditProject()));
    menuItem->setEnabled(editPriv);
    menuItem = pMenu->addAction(tr("View Project"), this, SLOT(sViewProject()));
    menuItem->setEnabled(viewPriv);
  }

  if (list()->altId() == 5  ||
      (list()->currentItem()->altId() == 1 &&
       list()->currentItem()->rawValue("parent") == "OPP"))
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalOpportunities")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalOpportunities")) ||
        (_privileges->check("MaintainAllOpportunities"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalOpportunities")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalOpportunities")) ||
        (_privileges->check("ViewAllOpportunities"));

    menuItem = pMenu->addAction(tr("Edit Opportunity"), this, SLOT(sEditOpportunity()));
    menuItem->setEnabled(editPriv);
    menuItem = pMenu->addAction(tr("View Opportunity"), this, SLOT(sViewOpportunity()));
    menuItem->setEnabled(viewPriv);
  }

  if (list()->currentItem()->rawValue("cust").toInt() > 0)
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
  else if (list()->altId() == 5)
    sEditOpportunity();
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
  else if (list()->altId() == 5)
    sViewOpportunity();
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
      !_opportunities->isChecked() &&
      !_incidents->isChecked() &&
      !_projects->isChecked())
  {
    list()->clear();
    return false;
  }

  if (_todolist->isChecked())
    params.append("todoList");
  if (_opportunities->isChecked())
    params.append("opportunities");
  if (_incidents->isChecked())
    params.append("incidents");
  if (_projects->isChecked())
    params.append("projects");

  params.append("todo", tr("To-do"));
  params.append("incident", tr("Incident"));
  params.append("task", tr("Task"));
  params.append("project", tr("Project"));
  params.append("opportunity", tr("Opportunity"));
  params.append("complete", tr("Completed"));
  params.append("deferred", tr("Deferred"));
  params.append("pending", tr("Pending"));
  params.append("inprocess", tr("InProcess"));
  params.append("feedback", tr("Feedback"));
  params.append("confirmed", tr("Confirmed"));
  params.append("assigned", tr("Assigned"));
  params.append("resolved", tr("Resolved"));
  params.append("closed", tr("Closed"));
  params.append("concept", tr("Concept"));
  params.append("new", tr("New"));

  if (!display::setParams(params))
    return false;

  return true;
}

int todoList::getId(int pType)
{
  if (list()->currentItem()->altId() == pType)
    return list()->id();
  else
    return list()->currentItem()->id("parent");
}

void todoList::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", getId(2));

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", getId(2));

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sNewProject()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEditProject()
{
  ParameterList params;
  params.append("mode", "edit");
  qDebug("project %d", getId(4));
  params.append("prj_id", getId(4));

  project newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewProject()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prj_id", getId(4));

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
  ParameterList params;
  params.append("cust_id", list()->rawValue("cust").toInt());
  params.append("mode","edit");

  customer *newdlg = new customer(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void todoList::sViewCustomer()
{
  ParameterList params;
  params.append("cust_id", list()->rawValue("cust").toInt());
  params.append("mode","view");

  customer *newdlg = new customer(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void todoList::sNewOpportunity()
{
  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");

  opportunity newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEditOpportunity()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("ophead_id", getId(5));

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewOpportunity()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ophead_id", getId(5));

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sOpen()
{
  bool editPriv = false;
  bool viewPriv = false;

  if (list()->currentItem()->altId() == 1)
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalToDoItems")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalToDoItems")) ||
        (_privileges->check("MaintainAllToDoItems"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalToDoItems")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalToDoItems")) ||
        (_privileges->check("ViewAllToDoItems"));

  }

  if (list()->altId() == 2 ||
      (list()->currentItem()->altId() == 1 &&
       list()->currentItem()->rawValue("parent") == "INCDT"))
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalIncidents")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalIncidents")) ||
        (_privileges->check("MaintainAllIncidents"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalIncidents")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalIncidents")) ||
        (_privileges->check("ViewAllIncidents"));

  }

  if (list()->altId() == 3)
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalProjects")) ||
        (_privileges->check("MaintainAllProjects"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalProjects")) ||
        (_privileges->check("ViewAllProjects"));

  }

  if (list()->altId() == 3 || list()->altId() == 4)
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalProjects")) ||
        (_privileges->check("MaintainAllProjects"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalProjects")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalProjects")) ||
        (_privileges->check("ViewAllProjects"));

  }

  if (list()->altId() == 5  ||
      (list()->currentItem()->altId() == 1 &&
       list()->currentItem()->rawValue("parent") == "OPP"))
  {
    editPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalOpportunities")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalOpportunities")) ||
        (_privileges->check("MaintainAllOpportunities"));

    viewPriv =
        (omfgThis->username() == list()->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalOpportunities")) ||
        (omfgThis->username() == list()->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalOpportunities")) ||
        (_privileges->check("ViewAllOpportunities"));

  }

  if(editPriv)
    sEdit();
  else if(viewPriv)
    sView();
  else
    QMessageBox::information(this, tr("Restricted Access"), tr("You have not been granted privileges to open this item."));
}


