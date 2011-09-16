/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedBookingsByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspBookingsByItem.h"

dspTimePhasedBookingsByItem::dspTimePhasedBookingsByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedBookingsByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Bookings by Item"));
  setListLabel(tr("Bookings"));
  setReportName("TimePhasedBookingsByItem");
  setMetaSQLOptions("timePhasedBookings", "detail");
  setUseAltId(true);

  _productCategory->setType(ParameterGroup::ProductCategory);

  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
}

void dspTimePhasedBookingsByItem::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedBookingsByItem::sViewBookings()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspBookingsByItem *newdlg = new dspBookingsByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedBookingsByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    menuItem = pMenu->addAction(tr("View Bookings..."), this, SLOT(sViewBookings()));
}

bool dspTimePhasedBookingsByItem::setParamsTP(ParameterList & params)
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
