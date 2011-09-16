/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
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

#include "characteristic.h"
#include "crmaccount.h"
#include "errorReporter.h"
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

  if (_privileges->check("MaintainAllCRMAccounts") || _privileges->check("ViewAllCRMAccounts"))
  {
    parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
    parameterWidget()->append(tr("Owner Pattern"), "owner_usr_pattern", ParameterWidget::Text);
  }
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

  connect(omfgThis, SIGNAL(crmAccountsUpdated(int)),     this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(employeeUpdated(int)),        this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(prospectsUpdated()),          this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(salesRepUpdated(int)),        this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)),        this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(userUpdated(QString)),        this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(vendorsUpdated()),            this, SLOT(sFillList()));

  list()->addColumn(tr("Number"),         80, Qt::AlignLeft,    true, "crmacct_number");
  list()->addColumn(tr("Name"),           -1, Qt::AlignLeft,    true, "crmacct_name");
  list()->addColumn(tr("Owner"), _userColumn, Qt::AlignLeft,   false, "crmacct_owner_username");
  list()->addColumn(tr("First"),          50, Qt::AlignLeft  ,  true, "cntct_first_name" );
  list()->addColumn(tr("Last"),           -1, Qt::AlignLeft  ,  true, "cntct_last_name" );
  list()->addColumn(tr("Phone"),         100, Qt::AlignLeft  ,  true, "cntct_phone" );
  list()->addColumn(tr("Email"),         100, Qt::AlignLeft  ,  true, "cntct_email" );
  list()->addColumn(tr("Address"),        -1, Qt::AlignLeft  , false, "addr_line1" );
  list()->addColumn(tr("City"),           75, Qt::AlignLeft  , false, "addr_city" );
  list()->addColumn(tr("State"),          50, Qt::AlignLeft  , false, "addr_state" );
  list()->addColumn(tr("Country"),       100, Qt::AlignLeft  , false, "addr_country" );
  list()->addColumn(tr("Postal Code"),    75, Qt::AlignLeft  , false, "addr_postalcode" );
  list()->addColumn(tr("Customer"),       70, Qt::AlignCenter,  true, "cust");
  list()->addColumn(tr("Prospect"),       70, Qt::AlignCenter,  true, "prospect");
  list()->addColumn(tr("Vendor"),         70, Qt::AlignCenter,  true, "vend");
  list()->addColumn(tr("Competitor"),     70, Qt::AlignCenter, false, "competitor");
  list()->addColumn(tr("Partner"),        70, Qt::AlignCenter, false, "partner");
  list()->addColumn(tr("Tax Auth."),      70, Qt::AlignCenter, false, "taxauth");
  list()->addColumn(tr("User"),           70, Qt::AlignCenter, false, "usr");
  list()->addColumn(tr("Employee"),       70, Qt::AlignCenter, false, "emp");
  list()->addColumn(tr("Sales Rep"),      70, Qt::AlignCenter, false, "salesrep");

  setupCharacteristics(characteristic::CRMAccounts);
  parameterWidget()->applyDefaultFilterSet();

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllCRMAccounts") && !_privileges->check("MaintainPersonalCRMAccounts"))
    newAction()->setEnabled(FALSE);
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
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("Are you sure you want to delete this CRM Account?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("DELETE FROM crmacct WHERE crmacct_id = :crmacct_id;");
  delq.bindValue(":crmacct_id", list()->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error deleting CRM Account"),
                           delq, __FILE__, __LINE__))
    return;
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

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("crmacct_owner_username") && _privileges->check("MaintainPersonalCRMAccounts")) ||
      (_privileges->check("MaintainAllCRMAccounts"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("crmacct_owner_username") && _privileges->check("ViewPersonalCRMAccounts")) ||
      (_privileges->check("ViewAllCRMAccounts"));

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);

  pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(editPriv);
}

void crmaccounts::sOpen()
{
  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("crmacct_owner_username") && _privileges->check("MaintainPersonalCRMAccounts")) ||
      (_privileges->check("MaintainAllCRMAccounts"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("crmacct_owner_username") && _privileges->check("ViewPersonalCRMAccounts")) ||
      (_privileges->check("ViewAllCRMAccounts"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();
}
