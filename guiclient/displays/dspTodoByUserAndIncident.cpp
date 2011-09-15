/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTodoByUserAndIncident.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "incident.h"
#include "todoItem.h"

dspTodoByUserAndIncident::dspTodoByUserAndIncident(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspTodoByUserAndIncident", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("To-Do List Items by User and Incident"));
  setReportName("TodoByUserAndIncident");
  setMetaSQLOptions("todoByUserAndIncident", "detail");

  _usr->setType(ParameterGroup::User);

  _dueDate->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dueDate->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _startDate->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _startDate->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Assigned To"),  _userColumn, Qt::AlignCenter,true, "todoitem_username");
  list()->addColumn(tr("Priority"),    _prcntColumn, Qt::AlignCenter,true, "incdtpriority_name");
  list()->addColumn(tr("Incident"),    _orderColumn, Qt::AlignLeft,  true, "incdt_number");
  list()->addColumn(tr("Task Name"),            100, Qt::AlignLeft,  true, "todoitem_name");
  list()->addColumn(tr("Status"),	 _statusColumn, Qt::AlignCenter,true, "todoitem_status");
  list()->addColumn(tr("Date Due"),     _dateColumn, Qt::AlignCenter,true, "todoitem_due_date");
  list()->addColumn(tr("Date Started"), _dateColumn, Qt::AlignCenter,true, "todoitem_start_date");
  list()->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "todoitem_description");

  _incident->setEnabled(_selectedIncident->isChecked());
}

void dspTodoByUserAndIncident::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspTodoByUserAndIncident::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditTodoItem()));
  menuItem->setEnabled(_privileges->check("MaintainOtherTodoLists"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewTodoItem()));
  menuItem->setEnabled(_privileges->check("ViewOtherTodoLists"));

  if (list()->altId() > 0)
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Edit Incident"), this, SLOT(sEditIncident()));
    menuItem->setEnabled(_privileges->check("MaintainIncidents"));

    menuItem = pMenu->addAction(tr("View Incident"), this, SLOT(sViewIncident()));
    menuItem->setEnabled(_privileges->check("ViewIncidents") ||
                         _privileges->check("MaintainIncidents"));
  }
}

bool dspTodoByUserAndIncident::setParams(ParameterList& params)
{
  if (_selectedIncident->isChecked() && _incident->id() <= 0)
  {
    QMessageBox::critical(this, tr("No Incident"),
			  tr("Please select an Incident before Querying."),
			  QMessageBox::Ok, QMessageBox::NoButton);
    _incident->setFocus();
    return false;
  }

  _usr->appendValue(params);
  if (_selectedIncident->isChecked())
    params.append("incdt_id", _incident->id());
  if (_showInactive->isChecked())
    params.append("showInactive");
  if (_showCompleted->isChecked())
    params.append("showCompleted");
  if (_startDate->startDate() > omfgThis->startOfTime())
    params.append("start_date_start", _startDate->startDate());
  if (_startDate->endDate() < omfgThis->endOfTime())
    params.append("start_date_end",   _startDate->endDate());
  if (_dueDate->startDate() > omfgThis->startOfTime())
    params.append("due_date_start",   _dueDate->startDate());
  if (_dueDate->endDate() < omfgThis->endOfTime())
    params.append("due_date_end",     _dueDate->endDate());

  return true;
}

void dspTodoByUserAndIncident::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", list()->altId());

  incident newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspTodoByUserAndIncident::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", list()->id());

  todoItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspTodoByUserAndIncident::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", list()->altId());

  incident newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
}

void dspTodoByUserAndIncident::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", list()->id());

  todoItem newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
}

