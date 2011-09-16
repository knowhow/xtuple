/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomersByCustomerType.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "customer.h"
#include "customerTypeList.h"

dspCustomersByCustomerType::dspCustomersByCustomerType(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspCustomersByCustomerType", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Customers by Customer Type"));
  setListLabel(tr("Customers"));
  setReportName("CustomersByCustomerType");
  setMetaSQLOptions("customer", "detail");
  setAutoUpdateEnabled(true);

  _customerType->setType(ParameterGroup::CustomerType);

  list()->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "custtype_code");
  list()->addColumn(tr("Active"),  _ynColumn,   Qt::AlignLeft,  true, "cust_active");
  list()->addColumn(tr("Number"),  _itemColumn, Qt::AlignLeft, true, "cust_number");
  list()->addColumn(tr("Name"),    200,         Qt::AlignLeft, true, "cust_name");
  list()->addColumn(tr("Address"), -1,          Qt::AlignLeft, true, "cust_address1");
  list()->setDragString("custid=");

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), SLOT(sFillList()));
}

void dspCustomersByCustomerType::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspCustomersByCustomerType::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("ViewCustomerMasters"));

  menuItem = pMenu->addAction("Reassign Customer Type...", this, SLOT(sReassignCustomerType()));
  menuItem->setEnabled(_privileges->check("MaintainCustomerMasters"));
}

void dspCustomersByCustomerType::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", list()->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCustomerType::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", list()->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCustomerType::sReassignCustomerType()
{
  ParameterList params;

  q.prepare( "SELECT cust_custtype_id FROM cust WHERE (cust_id=:cust_id)" );
  q.bindValue(":cust_id", list()->id());
  q.exec();
  if (q.first())
  {
    int _custTypeId = q.value("cust_custtype_id").toInt();
    params.append("custtype_id", _custTypeId);
  }

  customerTypeList *newdlg = new customerTypeList(this, "", true);
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
    omfgThis->sCustomersUpdated(list()->id(), true);
  }
}

bool dspCustomersByCustomerType::setParams(ParameterList &params)
{
  _customerType->appendValue(params);

  if(_showInactive->isChecked())
    params.append("showInactive");

  params.append("byCustType");  
 
  return true;
}
