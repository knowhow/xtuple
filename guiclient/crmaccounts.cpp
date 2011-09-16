/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccounts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmaccount.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

crmaccounts::crmaccounts(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "crmaccounts", fl)
{
  setWindowTitle(tr("Accounts"));
  setReportName("CRMAccountMasterList");
  setMetaSQLOptions("crmaccounts", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);
  parameterWidget()->append(tr("Account Number Pattern"), "crmacct_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Account Name Pattern"), "crmacct_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Contact Name Pattern"), "cntct_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Phone Pattern"), "cntct_phone_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Email Pattern"), "cntct_email_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Street Pattern"), "addr_street_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("City Pattern"), "addr_city_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("State Pattern"), "addr_state_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Postal Code Pattern"), "addr_postalcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Country Pattern"), "addr_country_pattern", ParameterWidget::Text);

  parameterWidget()->applyDefaultFilterSet();

  connect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(prospectsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(vendorsUpdated()), this, SLOT(sFillList()));

  list()->addColumn(tr("Number"),      80, Qt::AlignLeft,  true, "crmacct_number");
  list()->addColumn(tr("Name"),        -1, Qt::AlignLeft,  true, "crmacct_name");
  list()->addColumn(tr("First"),       50, Qt::AlignLeft  , true, "cntct_first_name" );
  list()->addColumn(tr("Last"),        -1, Qt::AlignLeft  , true, "cntct_last_name" );
  list()->addColumn(tr("Phone"),      100, Qt::AlignLeft  , true, "cntct_phone" );
  list()->addColumn(tr("Email"),      100, Qt::AlignLeft  , true, "cntct_email" );
  list()->addColumn(tr("Address"),     -1, Qt::AlignLeft  , false, "addr_line1" );
  list()->addColumn(tr("City"),        75, Qt::AlignLeft  , false, "addr_city" );
  list()->addColumn(tr("State"),       50, Qt::AlignLeft  , false, "addr_state" );
  list()->addColumn(tr("Country"),    100, Qt::AlignLeft  , false, "addr_country" );
  list()->addColumn(tr("Postal Code"), 75, Qt::AlignLeft  , false, "addr_postalcode" );
  list()->addColumn(tr("Customer"),    70, Qt::AlignCenter,true, "cust");
  list()->addColumn(tr("Prospect"),    70, Qt::AlignCenter,true, "prospect");
  list()->addColumn(tr("Vendor"),      70, Qt::AlignCenter,true, "vend");
  list()->addColumn(tr("Competitor"),  70, Qt::AlignCenter,false, "competitor");
  list()->addColumn(tr("Partner"),     70, Qt::AlignCenter,false, "partner");
  list()->addColumn(tr("Tax Auth."),   70, Qt::AlignCenter,false, "taxauth");

  if (_privileges->check("MaintainCRMAccounts"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(FALSE);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void crmaccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id", list()->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccounts::sDelete()
{
  q.prepare("SELECT deleteCRMAccount(:crmacct_id) AS returnVal;");
  q.bindValue(":crmacct_id", list()->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("returnVal").toInt();
    if (returnVal < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCRMAccount", returnVal),
		  __FILE__, __LINE__);
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

void crmaccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id", list()->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccounts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCRMAccounts"));

  pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainCRMAccounts"));
}


