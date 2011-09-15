/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedBookings.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <currcluster.h>

#include "guiclient.h"
#include "dspBookings.h"
#include "parameterwidget.h"

dspTimePhasedBookings::dspTimePhasedBookings(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedBookings", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Bookings"));
  setReportName("TimePhasedBookings");
  setMetaSQLOptions("timePhasedBookings", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Customer"),   "cust_id",   ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

  _groupBy->append(0,tr("Customer"));
  _groupBy->append(1,tr("Product Category"));
  _groupBy->append(2,tr("Item"));

  _units->append(0, tr("Sales Dollars"));
  _units->append(1, tr("Inventory"));
  _units->append(2,tr("Capacity"));
  _units->append(3, tr("Alt. Capacity"));

  connect(_groupBy, SIGNAL(newID(int)), this, SLOT(sGroupByChanged()));

  sGroupByChanged();
}

void dspTimePhasedBookings::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspTimePhasedBookings::sViewBookings()
{
  ParameterList params = parameterWidget()->parameters();
  if (_groupBy->id() == 1)
    params.append("prodcat_id", list()->id());
  else if (_groupBy->id() == 2)
    params.append("item_id", list()->id());
  else
    params.append("cust_id", list()->id());
  params.append("startDate", _columnDates[_column - 4].startDate);
  params.append("endDate", _columnDates[_column - 4].endDate);
  params.append("run");

  dspBookings *newdlg = new dspBookings();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedBookings::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  if (pColumn < 4)
    return;

  QAction *menuItem;

  _column = pColumn;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    menuItem = pMenu->addAction(tr("View Bookings..."), this, SLOT(sViewBookings()));
}

bool dspTimePhasedBookings::setParamsTP(ParameterList & params)
{
  parameterWidget()->appendValue(params);
  params.append("filter", parameterWidget()->filter());

  int idx = _groupBy->id();
  if (idx == 1)
    params.append("byProdcat");
  else if (idx == 2)
    params.append("byItem");
  else
    params.append("byCust");

  idx = _units->id();
  if (idx == 1)
    params.append("inventoryUnits");
  else if (idx == 2)
    params.append("capacityUnits");
  else if (idx == 3)
    params.append("altCapacityUnits");
  else
    params.append("salesDollars");

  params.append("baseCurrAbbr", CurrDisplay::baseCurrAbbr());

  return true;
}

void dspTimePhasedBookings::sGroupByChanged()
{
  int idx = _groupBy->id();

  list()->clear();
  list()->setColumnCount(0);

  if (idx == 1)
  {
    list()->addColumn(tr("Prod. Cat."), _itemColumn, Qt::AlignLeft,   true,  "prodcat_code"   );
    list()->addColumn(tr("Description"), 180,        Qt::AlignLeft,   true,  "prodcat_descrip" );
  }
  else if (idx == 2)
  {
    list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
    list()->addColumn(tr("Description"), 180,         Qt::AlignLeft,   true,  "item_descrip1" );
  }
  else
  {
    list()->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft,  true,  "cust_number" );
    list()->addColumn(tr("Name"), 180,              Qt::AlignLeft,  true,  "cust_name" );
  }

  list()->addColumn(tr("Site"),       _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("UOM"),        _uomColumn,  Qt::AlignLeft,   true,  "uom"   );
}
