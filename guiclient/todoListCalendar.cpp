/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoListCalendar.h"

#include "xdialog.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <calendargraphicsitem.h>
#include <metasql.h>
#include <openreports.h>

#include "todoCalendarControl.h"
#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "customer.h"

todoListCalendar::todoListCalendar(QWidget* parent, const char * name, Qt::WindowFlags f)
  : XWidget(parent, name, f)
{
  setupUi(this);

  todoCalendarControl * cc = new todoCalendarControl(this);
  QGraphicsScene * scene = new QGraphicsScene(this);
  calendar = new CalendarGraphicsItem(cc);
  calendar->setSelectedDay(QDate::currentDate());
  scene->addItem(calendar);

  _gview->setScene(scene);
  _gview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

  _usr->setEnabled(_privileges->check("MaintainAllToDoItems") || _privileges->check("MaintainPersonalToDoItems"));
  _usr->setType(ParameterGroup::User);
  q.prepare("SELECT getUsrId(NULL) AS usr_id;");
  q.exec();
  if (q.first())
  {
    _myUsrId = q.value("usr_id").toInt();
    _usr->setId(_myUsrId);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_active, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_completed, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_list, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_list, SIGNAL(itemSelectionChanged()), this, SLOT(handlePrivs()));
  connect(_usr, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_usr, SIGNAL(updated()), this, SLOT(handlePrivs()));


  _list->addColumn(tr("Type"),    _statusColumn,  Qt::AlignCenter, true, "type");
  _list->addColumn(tr("Seq"),        _seqColumn,  Qt::AlignRight,  true, "seq");
  _list->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  _list->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,  false, "owner");
  _list->addColumn(tr("Assigned"),  _userColumn,  Qt::AlignLeft,   true, "assigned");
  _list->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  _list->addColumn(tr("Description"),        -1,  Qt::AlignLeft,   true, "descrip");
  _list->addColumn(tr("Status"),  _statusColumn,  Qt::AlignLeft,   true, "status");
  _list->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  _list->addColumn(tr("Incident"), _orderColumn,  Qt::AlignLeft,   true, "incdt");
  _list->addColumn(tr("Customer"), _orderColumn,  Qt::AlignLeft,   true, "cust");
  _list->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _active->setChecked(true);

  _usr->setEnabled(_privileges->check("MaintainAllToDoItems") ||
                   _privileges->check("ViewAllToDoItems"));

  sFillList(QDate::currentDate());

  connect(_list, SIGNAL(itemSelected(int)), this, SLOT(sOpen()));
  connect(cc, SIGNAL(selectedDayChanged(QDate)), this, SLOT(sFillList(QDate)));
}

void todoListCalendar::languageChange()
{
  retranslateUi(this);
}

enum SetResponse todoListCalendar::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool           valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _usr->setId(param.toInt());
    sFillList();
  }
  return NoError;
}

void todoListCalendar::sOpen()
{
  bool editPriv =
        (omfgThis->username() == _list->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalToDoItems")) ||
        (omfgThis->username() == _list->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalToDoItems")) ||
        (_privileges->check("MaintainAllToDoItems"));

  bool viewPriv =
        (omfgThis->username() == _list->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalToDoItems")) ||
        (omfgThis->username() == _list->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalToDoItems")) ||
        (_privileges->check("ViewAllToDoItems"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();
}

void todoListCalendar::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  if (_list->currentItem()->text(0) == "T")
  {
    bool editPriv =
        (omfgThis->username() == _list->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalToDoItems")) ||
        (omfgThis->username() == _list->currentItem()->rawValue("assigned") && _privileges->check("MaintainPersonalToDoItems")) ||
        (_privileges->check("MaintainAllToDoItems"));

    bool viewPriv =
        (omfgThis->username() == _list->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalToDoItems")) ||
        (omfgThis->username() == _list->currentItem()->rawValue("assigned") && _privileges->check("ViewPersonalToDoItems")) ||
        (_privileges->check("ViewAllToDoItems"));

    menuItem = pMenu->addAction(tr("New..."), this, SLOT(sNew()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(editPriv);

    menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
    menuItem->setEnabled(viewPriv);

    menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
    menuItem->setEnabled(editPriv);
  }

  if (! _list->currentItem()->text(9).isEmpty())
  {
    menuItem = pMenu->addAction(tr("Edit Customer"), this, SLOT(sEditCustomer()));
    menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));
    menuItem = pMenu->addAction(tr("View Customer"), this, SLOT(sViewCustomer()));
    menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));
  }
}

void todoListCalendar::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_usr->isSelected())
    _usr->appendValue(params);

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoListCalendar::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _list->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoListCalendar::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _list->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoListCalendar::sDelete()
{
  q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  q.bindValue(":todoitem_id", _list->id());
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
      sFillList();
    }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void todoListCalendar::sEditCustomer()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", _list->currentItem()->text(9));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());
    params.append("mode","edit");

    customer *newdlg = new customer();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);

}

void todoListCalendar::sViewCustomer()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", _list->currentItem()->text(9));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());
    params.append("mode","view");

    customer *newdlg = new customer();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);

}

void todoListCalendar::setParams(ParameterList &params)
{
  if (_active->isChecked())
    params.append("active");
  if (_completed->isChecked())
    params.append("completed");
  _usr->appendValue(params);
}

void todoListCalendar::sFillList()
{
  sFillList(_lastDate);
}

void todoListCalendar::sFillList(const QDate & date)
{
  static bool dontBotherMe = false;
  if(dontBotherMe)
    return;
  dontBotherMe = true;
  _lastDate = date;
  calendar->setSelectedDay(_lastDate);
  QString sql = "SELECT todoitem_id AS id, todoitem_owner_username AS owner, "
                "       'T' AS type, incdtpriority_order AS seq, incdtpriority_name AS priority, "
                "       todoitem_name AS name, "
                "       firstLine(todoitem_description) AS descrip, "
                "       todoitem_status AS status, todoitem_due_date AS due, "
                "       todoitem_username AS assigned, todoitem_owner_username AS owner, "
                "       incdt_number AS incdt, cust_number AS cust, "
                "       CASE WHEN (todoitem_status != 'C'AND "
                "                  todoitem_due_date < CURRENT_DATE) THEN 'expired'"
                "            WHEN (todoitem_status != 'C'AND "
                "                  todoitem_due_date > CURRENT_DATE) THEN 'future'"
                "       END AS due_qtforegroundrole "
                "  FROM todoitem() LEFT OUTER JOIN incdt ON (incdt_id=todoitem_incdt_id) "
                "                  LEFT OUTER JOIN crmacct ON (crmacct_id=todoitem_crmacct_id) "
                "                  LEFT OUTER JOIN cust ON (cust_id=crmacct_cust_id) "
                "                  LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
                " WHERE( (todoitem_due_date = <? value(\"date\") ?>)"
                "  <? if not exists(\"completed\") ?>"
                "  AND   (todoitem_status != 'C')"
                "  <? endif ?>"
                "  <? if exists(\"username\") ?> "
                "  AND (todoitem_username=<? value(\"username\") ?>) "
                "  <? elseif exists(\"usr_pattern\") ?>"
                "  AND (todoitem_username ~ <? value(\"usr_pattern\") ?>) "
                "  <? endif ?>"
                "  <? if exists(\"active\") ?>AND (todoitem_active) <? endif ?>"
                "       ) "
                "ORDER BY due, seq, todoitem_username;";

  ParameterList params;
  params.append("date", date);
  setParams(params);

  MetaSQLQuery mql(sql);
  XSqlQuery itemQ = mql.toQuery(params);

  _list->populate(itemQ);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    dontBotherMe = false;
    return;
  }
  dontBotherMe = false;
}

void todoListCalendar::resizeEvent(QResizeEvent* event)
{
  XWidget::resizeEvent(event);

  _gview->setMinimumWidth((int)(_gview->height() * (_gview->scene()->sceneRect().width() / _gview->scene()->sceneRect().height())));
  _gview->fitInView(_gview->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void todoListCalendar::showEvent(QShowEvent * event)
{
  XWidget::showEvent(event);

  _gview->setMinimumWidth((int)(_gview->height() * (_gview->scene()->sceneRect().width() / _gview->scene()->sceneRect().height())));
  _gview->fitInView(_gview->scene()->sceneRect(), Qt::KeepAspectRatio);
}
