/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByProductCategory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspSalesHistoryByParameterList.h"
#include "guiclient.h"

dspTimePhasedSalesByProductCategory::dspTimePhasedSalesByProductCategory(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedSalesByProductCategory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Sales History by Product Category"));
  setListLabel(tr("Sales History"));
  setReportName("TimePhasedSalesHistoryByProductCategory");
  setMetaSQLOptions("timePhasedSales", "detail");
  setUseAltId(true);

  _productCategory->setType(ParameterGroup::ProductCategory);
  
  list()->addColumn(tr("Prod. Cat."), _itemColumn, Qt::AlignLeft,   true,  "prodcat_code"   );
  list()->addColumn(tr("UOM"),        _uomColumn,  Qt::AlignCenter, true,  "uom" );
  list()->addColumn(tr("Site"),       _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );

  _salesDollars->setEnabled(_privileges->check("ViewCustomerPrices"));
}

void dspTimePhasedSalesByProductCategory::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedSalesByProductCategory::sViewShipments()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("prodcat_id", list()->id());
    params.append("warehous_id", list()->altId());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedSalesByProductCategory::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  menuItem = menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewShipments()));
  menuItem->setEnabled(_privileges->check("ViewSalesHistory"));
}

bool dspTimePhasedSalesByProductCategory::setParamsTP(ParameterList & params)
{
  params.append("byProdcat");

  if (_salesDollars->isChecked())
    params.append("salesDollars");
  else if (_inventoryUnits->isChecked())
    params.append("inventoryUnits");
  else if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else if (_altCapacityUnits->isChecked())
    params.append("altCapacityUnits");

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);

  return true;
}
