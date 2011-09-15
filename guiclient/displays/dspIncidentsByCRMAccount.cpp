/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspIncidentsByCRMAccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "crmaccount.h"
#include "incident.h"
#include "todoItem.h"
#include "mqlutil.h"

dspIncidentsByCRMAccount::dspIncidentsByCRMAccount(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspIncidentsByCRMAccount", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Incidents by CRM Account"));
  setListLabel(tr("Incidents"));
  setReportName("IncidentsByCRMAccount");
  setMetaSQLOptions("incidentsbyCRMAccount", "detail");
  setUseAltId(true);

  QButtonGroup* _crmacctGroupInt = new QButtonGroup(this);
  _crmacctGroupInt->addButton(_allAccts);
  _crmacctGroupInt->addButton(_selectedAcct);

  _createdDate->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _createdDate->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Account Number"),	    80, Qt::AlignLeft,  true, "crmacct_number");
  list()->addColumn(tr("Account Name"),		   100, Qt::AlignLeft,  true, "crmacct_name");
  list()->addColumn(tr("Incident"),	  _orderColumn, Qt::AlignRight, true, "incdt_number");
  list()->addColumn(tr("Summary"),	            -1, Qt::AlignLeft,  true, "summary");
  list()->addColumn(tr("Entered/Assigned"), _dateColumn, Qt::AlignLeft,  true, "startdate");
  list()->addColumn(tr("Status"),         _statusColumn, Qt::AlignCenter,true, "status");
  list()->addColumn(tr("Assigned To"),	   _userColumn, Qt::AlignLeft,  true, "assigned");
  list()->addColumn(tr("To-Do Due"),	   _dateColumn, Qt::AlignLeft,  true, "duedate");

  list()->setIndentation(10);
}

void dspIncidentsByCRMAccount::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspIncidentsByCRMAccount::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _selectedAcct->setChecked(true);
    _crmacct->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspIncidentsByCRMAccount::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit CRM Account..."), this, SLOT(sEditCRMAccount()));
    menuItem->setEnabled(_privileges->check("MaintainCRMAccounts"));
    menuItem = pMenu->addAction(tr("View CRM Account..."), this, SLOT(sViewCRMAccount()));
    menuItem->setEnabled( _privileges->check("ViewCRMAccounts") ||
				    _privileges->check("MaintainCRMAccounts"));
  }
  else if (list()->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Edit Incident..."), this, SLOT(sEditIncident()));
    menuItem->setEnabled(_privileges->check("MaintainIncidents"));
    menuItem = pMenu->addAction(tr("View Incident..."), this, SLOT(sViewIncident()));
    menuItem->setEnabled( _privileges->check("ViewIncidents") ||
				    _privileges->check("MaintainIncidents"));
  }
  else if (list()->altId() == 3)
  {
    menuItem = pMenu->addAction(tr("Edit To-Do Item..."), this, SLOT(sEditTodoItem()));
    menuItem->setEnabled(_privileges->check("MaintainOtherTodoLists"));

    menuItem = pMenu->addAction(tr("View To-Do Item..."), this, SLOT(sViewTodoItem()));
    menuItem->setEnabled(_privileges->check("ViewOtherTodoLists"));
  }
}

void dspIncidentsByCRMAccount::sEditCRMAccount()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id", list()->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void dspIncidentsByCRMAccount::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", list()->id());

  incident newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspIncidentsByCRMAccount::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", list()->id());

  todoItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspIncidentsByCRMAccount::sViewCRMAccount()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id", list()->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void dspIncidentsByCRMAccount::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", list()->id());

  incident newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
}

void dspIncidentsByCRMAccount::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", list()->id());

  todoItem newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
}

bool dspIncidentsByCRMAccount::setParams(ParameterList &params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  if (_selectedAcct->isChecked() && _crmacct->id() <= 0)
  {
    QMessageBox::critical(this, tr("No CRM Account"),
			  tr("Please select a CRM Account before Querying or Printing."),
			  QMessageBox::Ok, QMessageBox::NoButton);
    _crmacct->setFocus();
    return false;
  }

  if (! _allAccts->isChecked())
    params.append("crmacct_id", _crmacct->id());

  if (_showClosed->isChecked())
    params.append("showClosed");

  if (_showAcctsWOIncdts->isChecked())
    params.append("showAcctsWOIncdts");

  _createdDate->appendValue(params);

  return true;
}

