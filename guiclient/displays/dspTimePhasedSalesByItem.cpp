/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspSalesHistoryByItem.h"
#include "guiclient.h"

dspTimePhasedSalesByItem::dspTimePhasedSalesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedSalesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Sales History by Item"));
  setListLabel(tr("Sales History"));
  setReportName("TimePhasedSalesHistoryByItem");
  setMetaSQLOptions("timePhasedSales", "detail");

  _productCategory->setType(ParameterGroup::ProductCategory);
  
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter, true,  "uom" );
  list()->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );

  _salesDollars->setEnabled(_privileges->check("ViewCustomerPrices"));
}

void dspTimePhasedSalesByItem::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedSalesByItem::sViewShipments()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspSalesHistoryByItem *newdlg = new dspSalesHistoryByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedSalesByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;
  _column = pColumn;

  menuItem = menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewShipments()));
  menuItem->setEnabled(_privileges->check("ViewSalesHistory"));
}

bool dspTimePhasedSalesByItem::setParamsTP(ParameterList & params)
{
  params.append("byItem");

  if (_salesDollars->isChecked())
    params.append("salesDollars");
  else if (_inventoryUnits->isChecked())
    params.append("inventoryUnits");

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);

  return true;
}
