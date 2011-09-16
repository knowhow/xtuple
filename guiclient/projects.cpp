/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projects.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <parameter.h>

#include "parameterwidget.h"
#include "project.h"

projects::projects(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "projects", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Projects"));
  setMetaSQLOptions("projects", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllProjects") && !_privileges->check("MaintainPersonalProjects"))
    newAction()->setEnabled(FALSE);

  list()->addColumn(tr("Number"),    _orderColumn, Qt::AlignLeft,   true, "prj_number" );
  list()->addColumn(tr("Name"),      -1,           Qt::AlignLeft,   true, "prj_name"   );
  list()->addColumn(tr("Status"),    _itemColumn,  Qt::AlignCenter, true, "prj_status" );
  list()->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"prj_owner_username");
  list()->addColumn(tr("Assigned"),  _userColumn,  Qt::AlignLeft,   true, "prj_username");
  list()->addColumn(tr("Budget Hrs."),        -1,  Qt::AlignRight,  true, "budget_hrs");
  list()->addColumn(tr("Actual Hrs."),        -1,  Qt::AlignRight,  true, "actual_hrs");
  list()->addColumn(tr("Balance Hrs."),       -1,  Qt::AlignRight,  true, "balance_hrs");

  connect(omfgThis, SIGNAL(projectsUpdated(int)), this, SLOT(sFillList()));
  connect(_showComplete, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("AssignedTo"), "assigned_username", ParameterWidget::User);

  sFillList();
}

void projects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sEdit()
{
  ParameterList params;
  params.append("prj_id", list()->id());
  params.append("mode", "edit");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sView()
{
  ParameterList params;
  params.append("prj_id", list()->id());
  params.append("mode", "view");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void projects::sDelete()
{
  q.prepare("SELECT deleteProject(:prj_id) AS result");
  q.bindValue(":prj_id", list()->id());
  q.exec();
  if(q.first())
  {
    int result = q.value("result").toInt();
    if(result < 0)
    {
      QString errmsg;
      switch(result)
      {
        case -1:
          errmsg = tr("One or more Quote's refer to this project.");
          break;
        case -2:
          errmsg = tr("One or more Sales Orders refer to this project.");
          break;
        case -3:
          errmsg = tr("One or more Work Orders refer to this project.");
          break;
        case -4:
          errmsg = tr("One or more Purchase Requests refer to this project.");
          break;
        case -5:
          errmsg = tr("One or more Purchase order Items refer to this project.");
          break;
        case -6:
          errmsg = tr("One or more Invoices refer to this project.");
          break;
        default:
          errmsg = tr("Error #%1 encountered while trying to delete project.").arg(result);
      }
      QMessageBox::critical( this, tr("Cannot Delete Project"),
        tr("Could not delete the project for one or more reasons.\n") + errmsg);
      return;
    }
  }
  sFillList();
}

void projects::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("MaintainPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("MaintainPersonalProjects")) ||
      (_privileges->check("MaintainAllProjects"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("ViewPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("ViewPersonalProjects")) ||
      (_privileges->check("ViewAllProjects"));

  menuItem = pMenu->addAction("View...", this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction("Delete...", this, SLOT(sDelete()));
  menuItem->setEnabled(editPriv);
}

bool projects::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (_showComplete->isChecked())
    params.append("showComplete",true);

  params.append("planning", tr("Concept"));
  params.append("open", tr("In-Process"));
  params.append("complete", tr("Complete"));
  params.append("undefined", tr("Undefined"));

  return true;
}

void projects::sOpen()
{
  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("MaintainPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("MaintainPersonalProjects")) ||
      (_privileges->check("MaintainAllProjects"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("ViewPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("ViewPersonalProjects")) ||
      (_privileges->check("ViewAllProjects"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();

}

