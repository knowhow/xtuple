/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByCustomerByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspSalesHistoryByCustomer.h"
#include "guiclient.h"

dspTimePhasedSalesByCustomerByItem::dspTimePhasedSalesByCustomerByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedSalesByCustomerByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Sales History by Customer by Item"));
  setListLabel(tr("Sales History"));
  setReportName("TimePhasedSalesHistoryByCustomerByItem");
  setMetaSQLOptions("timePhasedSalesByCustomerByItem", "detail");

  _productCategory->setType(ParameterGroup::ProductCategory);
  
  list()->addColumn(tr("Item #"),  _orderColumn, Qt::AlignLeft,   true,  "item_number" );
  list()->addColumn(tr("Description"), 180,          Qt::AlignLeft,   true,  "item_descrip1" );
}

void dspTimePhasedSalesByCustomerByItem::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedSalesByCustomerByItem::sViewShipments()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
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

void dspTimePhasedSalesByCustomerByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (pColumn > 1)
  {
    menuItem = menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewShipments()));
    menuItem->setEnabled(_privileges->check("ViewSalesHistory"));
  }
}

bool dspTimePhasedSalesByCustomerByItem::setParamsTP(ParameterList & params)
{
  if (!_cust->isValid())
  {
    return false;
  }

  params.append("cust_id", _cust->id());

  _productCategory->appendValue(params);

  params.append("orderByCustomer");

  return true;
}

