/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customers.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "customer.h"
#include "customerTypeList.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

customers::customers(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "customers", fl)
{
  setUseAltId(true);
  setWindowTitle(tr("Customers"));
  setMetaSQLOptions("customers", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);
  parameterWidget()->append(tr("Customer Number Pattern"), "cust_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Customer Name Pattern"), "cust_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_code_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Contact Name Pattern"), "cntct_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Phone Pattern"), "cntct_phone_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Email Pattern"), "cntct_email_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Street Pattern"), "addr_street_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("City Pattern"), "addr_city_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("State Pattern"), "addr_state_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Postal Code Pattern"), "addr_postalcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Country Pattern"), "addr_country_pattern", ParameterWidget::Text);

  if (_privileges->check("MaintainCustomerMasters"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  list()->addColumn(tr("Number"),  _orderColumn, Qt::AlignLeft, true, "cust_number");
  list()->addColumn(tr("Active"),  _ynColumn,    Qt::AlignCenter, false, "cust_active");
  list()->addColumn(tr("Name"),    -1,           Qt::AlignLeft,   true, "cust_name");
  list()->addColumn(tr("Type"),    _itemColumn,  Qt::AlignLeft, true, "custtype_code");
  list()->addColumn(tr("First"),   50, Qt::AlignLeft  , true, "cntct_first_name" );
  list()->addColumn(tr("Last"),    -1, Qt::AlignLeft  , true, "cntct_last_name" );
  list()->addColumn(tr("Phone"),   100, Qt::AlignLeft  , true, "cntct_phone" );
  list()->addColumn(tr("Email"),   100, Qt::AlignLeft  , true, "cntct_email" );
  list()->addColumn(tr("Address"), -1, Qt::AlignLeft  , false, "addr_line1" );
  list()->addColumn(tr("City"),    75, Qt::AlignLeft  , false, "addr_city" );
  list()->addColumn(tr("State"),   50, Qt::AlignLeft  , false, "addr_state" );
  list()->addColumn(tr("Country"), 100, Qt::AlignLeft  , false, "addr_country" );
  list()->addColumn(tr("Postal Code"), 75, Qt::AlignLeft  , false, "addr_postalcode" );

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), SLOT(sFillList()));
}

void customers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sEdit()
{
  ParameterList params;
  params.append("cust_id", list()->id());
  params.append("mode", "edit");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sView()
{
  ParameterList params;
  params.append("cust_id", list()->id());
  params.append("mode", "view");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sReassignCustomerType()
{
  ParameterList params;
  params.append("custtype_id", list()->altId());

  customerTypeList *newdlg = new customerTypeList(this, "", TRUE);
  newdlg->set(params);
  int custtypeid = newdlg->exec();
  if ( (custtypeid != -1) && (custtypeid != XDialog::Rejected) )
  {
    q.prepare( "UPDATE custinfo "
               "SET cust_custtype_id=:custtype_id "
               "WHERE (cust_id=:cust_id);" );
    q.bindValue(":cust_id", list()->id());
    q.bindValue(":custtype_id", custtypeid);
    q.exec();
    omfgThis->sCustomersUpdated(list()->id(), TRUE);
    sFillList();
  }
}

void customers::sDelete()
{
  if ( QMessageBox::warning(this, tr("Delete Customer?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected Customer?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteCustomer(:cust_id) AS result;");
    q.bindValue(":cust_id", list()->id());
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("result").toInt();
      if (returnVal < 0)
      {
        QMessageBox::critical(this, tr("Cannot Delete Customer"),
                              storedProcErrorLookup("deleteCustomer", returnVal));
        return;
      }
      omfgThis->sCustomersUpdated(-1, TRUE);
      sFillList();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void customers::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("View...", this, SLOT(sView()));

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));

  menuItem = pMenu->addAction("Delete", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction("Reassign Customer Type", this, SLOT(sReassignCustomerType()));
  menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));

}

