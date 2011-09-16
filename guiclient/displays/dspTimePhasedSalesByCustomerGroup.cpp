/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByCustomerGroup.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspSalesHistoryByCustomer.h"
#include "guiclient.h"

dspTimePhasedSalesByCustomerGroup::dspTimePhasedSalesByCustomerGroup(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedSalesByCustomerGroup", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Sales History by Customer Group"));
  setListLabel(tr("Sales History"));
  setReportName("TimePhasedSalesHistoryByCustomerGroup");
  setMetaSQLOptions("timePhasedSalesByCustomer", "detail");

  _customerGroup->setType(ParameterGroup::CustomerGroup);
  _productCategory->setType(ParameterGroup::ProductCategory);
  
  list()->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft,   true,  "cust_number" );
  list()->addColumn(tr("Customer"), 180,          Qt::AlignLeft,   true,  "cust_name" );
}

void dspTimePhasedSalesByCustomerGroup::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedSalesByCustomerGroup::sViewShipments()
{
  ParameterList params;
  params.append("cust_id", list()->id());
  params.append("startDate", _columnDates[_column - 2].startDate);
  params.append("endDate", _columnDates[_column - 2].endDate);
  params.append("run");

  if (_productCategory->isSelected())
    params.append("prodcat_id", _productCategory->id());
  else if (_productCategory->isPattern())
    params.append("prodcat_pattern", _productCategory->pattern());

  dspSalesHistoryByCustomer *newdlg = new dspSalesHistoryByCustomer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedSalesByCustomerGroup::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (pColumn > 1)
  {
    menuItem = menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewShipments()));
    menuItem->setEnabled(_privileges->check("ViewSalesHistory"));
  }
}

bool dspTimePhasedSalesByCustomerGroup::setParamsTP(ParameterList & params)
{
  _productCategory->appendValue(params);
  _customerGroup->appendValue(params);

  params.append("byCustomerGroup"); // are these two different?
  params.append("orderByCustomer"); // are these two different?

  return true;
}

