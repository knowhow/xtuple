/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspSalesHistoryByCustomer.h"
#include "guiclient.h"

dspTimePhasedSalesByCustomer::dspTimePhasedSalesByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedSalesByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Sales History by Customer"));
  setListLabel(tr("Sales History"));
  setReportName("TimePhasedSalesHistoryByCustomer");
  setMetaSQLOptions("timePhasedSalesByCustomer", "detail");

  _customerType->setType(ParameterGroup::CustomerType);
  _productCategory->setType(ParameterGroup::ProductCategory);
  
  list()->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft,   true,  "cust_number" );
  list()->addColumn(tr("Customer"), 180,          Qt::AlignLeft,   true,  "cust_name" );
}

void dspTimePhasedSalesByCustomer::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedSalesByCustomer::sViewShipments()
{
  ParameterList params;
  params.append("cust_id", list()->id());
  params.append("startDate", _columnDates[_column - 2].startDate);
  params.append("endDate", _columnDates[_column - 2].endDate);
  params.append("run");
  _productCategory->appendValue(params);

  dspSalesHistoryByCustomer *newdlg = new dspSalesHistoryByCustomer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedSalesByCustomer::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (pColumn > 1)
  {
    menuItem = menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewShipments()));
    menuItem->setEnabled(_privileges->check("ViewSalesHistory"));
  }
}

bool dspTimePhasedSalesByCustomer::setParamsTP(ParameterList & params)
{
  _productCategory->appendValue(params);
  _customerType->appendValue(params);

  params.append("orderByCustomer");

  return true;
}

