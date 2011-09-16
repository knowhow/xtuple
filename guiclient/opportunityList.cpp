/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityList.h"

#include "xdialog.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QToolBar>
#include <QVariant>

#include "guiclient.h"
#include "opportunity.h"
#include "parameterwidget.h"
#include "storedProcErrorLookup.h"

opportunityList::opportunityList(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "opportunityList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Opportunities"));
  setReportName("OpportunityList");
  setMetaSQLOptions("opportunities", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  list()->addColumn(tr("Active"),      _orderColumn,    Qt::AlignLeft,   false, "ophead_active" );
  list()->addColumn(tr("Name"),        -1,              Qt::AlignLeft,   true, "ophead_name"  );
  list()->addColumn(tr("CRM Acct."),   _userColumn,     Qt::AlignLeft,   true, "crmacct_number" );
  list()->addColumn(tr("Owner"),       _userColumn,     Qt::AlignLeft,   true, "ophead_owner_username" );
  list()->addColumn(tr("Stage"),       _orderColumn,    Qt::AlignLeft,   true, "opstage_name" );
  list()->addColumn(tr("Source"),      _orderColumn,    Qt::AlignLeft,   false, "opsource_name" );
  list()->addColumn(tr("Type"),        _orderColumn,    Qt::AlignLeft,   false, "optype_name" );
  list()->addColumn(tr("Prob.%"),      _prcntColumn,    Qt::AlignCenter, false, "ophead_probability_prcnt" );
  list()->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  false, "ophead_amount" );
  list()->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   false, "f_currency" );
  list()->addColumn(tr("Target Date"), _dateColumn,     Qt::AlignLeft,   false, "ophead_target_date" );
  list()->addColumn(tr("Actual Date"), _dateColumn,     Qt::AlignLeft,   false, "ophead_actual_date" );

  connect(list(),       SIGNAL(itemSelected(int)), this, SLOT(sEdit()));

  bool canEditUsers = _privileges->check("MaintainOtherTodoLists");
  parameterWidget()->append(tr("User"), "username", ParameterWidget::User, omfgThis->username(), !canEditUsers);
  if (canEditUsers)
    parameterWidget()->append(tr("User Pattern"), "usr_pattern",    ParameterWidget::Text);
  else
    parameterWidget()->setEnabled(tr("User"), false);
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date);
  parameterWidget()->append(tr("CRM Account"), "crmacct_id",  ParameterWidget::Crmacct);
  parameterWidget()->appendComboBox(tr("Type"), "optype_id", XComboBox::OpportunityTypes);
  parameterWidget()->append(tr("Type Pattern"), "optype_pattern",    ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Source"), "opsource_id", XComboBox::OpportunitySources);
  parameterWidget()->append(tr("Source Pattern"), "source_pattern",    ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Stage"), "opstage_id", XComboBox::OpportunityStages);
  parameterWidget()->append(tr("Stage Pattern"), "opstage_pattern",    ParameterWidget::Text);

  parameterWidget()->applyDefaultFilterSet();
}

void opportunityList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv = _privileges->check("MaintainOpportunities");
  bool viewPriv = _privileges->check("VeiwOpportunities") || editPriv;

  menuItem = pMenu->addAction(tr("New..."), this, SLOT(sNew()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(editPriv);

  pMenu->addSeparator();

  if (list()->altId() == 0)
  {
    menuItem = pMenu->addAction(tr("Deactivate"), this, SLOT(sDeactivate()));
    menuItem->setEnabled(editPriv);
  }
  else
  {
    menuItem = pMenu->addAction(tr("Activate"), this, SLOT(sActivate()));
    menuItem->setEnabled(editPriv);
  }
}

enum SetResponse opportunityList::set(const ParameterList& pParams)
{
  XWidget::set(pParams);

  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void opportunityList::sNew()
{
  ParameterList params;
  setParams(params);
  params.append("mode","new");

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ophead_id", list()->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ophead_id", list()->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void opportunityList::sDelete()
{
  q.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");
  q.bindValue(":ophead_id", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteOpportunity", result));
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

void opportunityList::sDeactivate()
{
  q.prepare("UPDATE ophead SET ophead_active=false WHERE ophead_id=:ophead_id;");
  q.bindValue(":ophead_id", list()->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  else
    sFillList();
}

void opportunityList::sActivate()
{
  q.prepare("UPDATE ophead SET ophead_active=true WHERE ophead_id=:ophead_id;");
  q.bindValue(":ophead_id", list()->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  else
    sFillList();
}

bool opportunityList::setParams(ParameterList &params)
{
  parameterWidget()->appendValue(params);

  if (!searchText().isEmpty())
    params.append("searchpattern", searchText());

  if (!findChild<QCheckBox*>("_showInactive")->isChecked())
    params.append("activeOnly");

  return true;
}


