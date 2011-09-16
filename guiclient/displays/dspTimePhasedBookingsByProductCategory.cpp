/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedBookingsByProductCategory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspBookingsByProductCategory.h"

dspTimePhasedBookingsByProductCategory::dspTimePhasedBookingsByProductCategory(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedBookingsByProductCategory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Bookings by Product Category"));
  setListLabel(tr("Bookings"));
  setReportName("TimePhasedBookingsByProductCategory");
  setMetaSQLOptions("timePhasedBookings", "detail");
  setUseAltId(true);

  _productCategory->setType(ParameterGroup::ProductCategory);

  list()->addColumn(tr("Prod. Cat."), _itemColumn, Qt::AlignLeft,   true,  "prodcat_code"   );
  list()->addColumn(tr("Site"),       _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("UOM"),        _uomColumn,  Qt::AlignLeft,   true,  "uom"   );
}

void dspTimePhasedBookingsByProductCategory::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedBookingsByProductCategory::sViewBookings()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("prodcat_id", list()->id());
    params.append("warehous_id", list()->altId());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspBookingsByProductCategory *newdlg = new dspBookingsByProductCategory();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedBookingsByProductCategory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    menuItem = pMenu->addAction(tr("View Bookings..."), this, SLOT(sViewBookings()));
}

bool dspTimePhasedBookingsByProductCategory::setParamsTP(ParameterList & params)
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
