/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentWorkbench.h"

#include <QSqlError>

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
  setNewVisible(true);
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
  parameterWidget()->append(tr("Assigned User"), "assigned_username", ParameterWidget::User, omfgThis->username());
  parameterWidget()->append(tr("Assigned Pattern"), "assigned_usr_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("Owner Pattern"), "owner_usr_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"), "endDate", ParameterWidget::Date);

  parameterWidget()->applyDefaultFilterSet();

  connect(list(),       SIGNAL(itemSelected(int)), this, SLOT(sEdit()));

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

  parameterWidget()->appendValue(params);

  params.append("pattern", searchText());

  return true;
}

void incidentWorkbench::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  pMenu->addAction(tr("Edit"), this, SLOT(sEdit()));
  pMenu->addAction(tr("View"), this, SLOT(sView()));
}


