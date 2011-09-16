/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentWorkbench.h"

#include <QSqlError>

#include "characteristic.h"
#include "guiclient.h"
#include "incident.h"
#include "parameterwidget.h"

incidentWorkbench::incidentWorkbench(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "incidentWorkbench", fl)
{
  setWindowTitle(tr("Incidents"));
  setListLabel(tr("Incidents"));
  setReportName("IncidentWorkbenchList");
  setMetaSQLOptions("incidents", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(_privileges->check("MaintainAllIncidents") || _privileges->check("MaintainPersonalIncidents"));
  setSearchVisible(true);
  setQueryOnStartEnabled(true);
  setAutoUpdateEnabled(true);

  QString qryStatus = QString("SELECT status_seq, "
                              " CASE WHEN status_code = 'N' THEN '%1' "
                              " WHEN status_code = 'F' THEN '%2' "
                              " WHEN status_code = 'C' THEN '%3' "
                              " WHEN status_code = 'A' THEN '%4' "
                              " WHEN status_code = 'R' THEN '%5' "
                              " WHEN status_code = 'L' THEN '%6' "
                              " END AS name, status_code AS code "
                              "FROM status; ")
      .arg(tr("New"))
      .arg(tr("Feedback"))
      .arg(tr("Confirmed"))
      .arg(tr("Assigned"))
      .arg(tr("Resolved"))
      .arg(tr("Closed"));

  QString qryPriority = "SELECT incdtpriority_id, incdtpriority_name "
                        "FROM incdtpriority "
                        "ORDER BY incdtpriority_order, incdtpriority_name ";

  parameterWidget()->append(tr("CRM Account"), "crmAccountId", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Contact"),"cntct_id", ParameterWidget::Contact);
  parameterWidget()->append(tr("Category"), "categorylist",
                           ParameterWidget::Multiselect, QVariant(), false,
                           "SELECT incdtcat_id, incdtcat_name"
                           "  FROM incdtcat"
                           " ORDER BY incdtcat_name;");
  parameterWidget()->appendComboBox(tr("Status Above"), "status_above", qryStatus, 4);
  parameterWidget()->append(tr("Status"), "statuslist",
                           ParameterWidget::Multiselect, QVariant(), false,
                           qryStatus);
  parameterWidget()->appendComboBox(tr("Severity"), "severity_id", XComboBox::IncidentSeverity);
  parameterWidget()->append(tr("User"), "username", ParameterWidget::User, omfgThis->username());
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("Assigned To"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"), "endDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Priority"), "incdtpriority_id_list", ParameterWidget::Multiselect, QVariant(), false, qryPriority);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);
  if(_metrics->boolean("IncidentsPublicPrivate"))
    parameterWidget()->append(tr("Public"), "public", ParameterWidget::CheckBox);
  parameterWidget()->applyDefaultFilterSet();

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllIncidents") && !_privileges->check("MaintainPersonalIncidents"))
    newAction()->setEnabled(FALSE);

  list()->addColumn(tr("Number"),      _orderColumn,Qt::AlignLeft, true, "incdt_number" );
  list()->addColumn(tr("Created"),     _dateColumn, Qt::AlignLeft, true, "incdt_timestamp" );
  list()->addColumn(tr("Account"),     _itemColumn, Qt::AlignLeft, true, "crmacct_name" );
  list()->addColumn(tr("Status"),      _itemColumn, Qt::AlignLeft, true, "incdt_status" );
  list()->addColumn(tr("Updated"),     _dateColumn, Qt::AlignLeft, true, "incdt_updated" );
  list()->addColumn(tr("Assigned To"), _userColumn, Qt::AlignLeft, true, "incdt_assigned_username" );
  list()->addColumn(tr("Owner"),       _userColumn, Qt::AlignLeft, true, "incdt_owner_username" );
  list()->addColumn(tr("Summary"),     -1,          Qt::AlignLeft, true, "incdt_summary" );
  list()->addColumn(tr("Category"),    _userColumn, Qt::AlignLeft, false, "incdtcat_name");
  list()->addColumn(tr("Severity"),    _userColumn, Qt::AlignLeft, false, "incdtseverity_name");
  list()->addColumn(tr("Priority"),    _userColumn, Qt::AlignLeft, false, "incdtpriority_name");
  list()->addColumn(tr("Contact"),     _userColumn, Qt::AlignLeft, false, "cntct_name");
  list()->addColumn(tr("Project"),     _userColumn, Qt::AlignLeft, false, "prj_number");
  if(_metrics->boolean("IncidentsPublicPrivate"))
    list()->addColumn(tr("Public"),     _userColumn, Qt::AlignLeft, false, "incdt_public");

  setupCharacteristics(characteristic::Incidents);
  parameterWidget()->applyDefaultFilterSet();
}

enum SetResponse incidentWorkbench::set(const ParameterList &pParams)
{
  XWidget::set(pParams);

  QVariant param;
  bool     valid;

  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void incidentWorkbench::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentWorkbench::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", list()->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentWorkbench::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", list()->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

bool incidentWorkbench::setParams(ParameterList & params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  params.append("startDate", omfgThis->startOfTime());
  params.append("endDate", omfgThis->endOfTime());

  if (!display::setParams(params))
    return false;

  return true;
}

void incidentWorkbench::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_owner_username") && _privileges->check("MaintainPersonalIncidents")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_assigned_username") && _privileges->check("MaintainPersonalIncidents")) ||
      (_privileges->check("MaintainAllIncidents"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_owner_username") && _privileges->check("ViewPersonalIncidents")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_username") && _privileges->check("ViewPersonalIncidents")) ||
      (_privileges->check("ViewAllIncidents"));

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);
  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);
}

void incidentWorkbench::sOpen()
{
  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_owner_username") && _privileges->check("MaintainPersonalIncidents")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_assigned_username") && _privileges->check("MaintainPersonalIncidents")) ||
      (_privileges->check("MaintainAllIncidents"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_owner_username") && _privileges->check("ViewPersonalIncidents")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("incdt_username") && _privileges->check("ViewPersonalIncidents")) ||
      (_privileges->check("ViewAllIncidents"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();
}


