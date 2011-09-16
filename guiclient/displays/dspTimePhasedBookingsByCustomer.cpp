/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedBookingsByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspBookingsByCustomer.h"

dspTimePhasedBookingsByCustomer::dspTimePhasedBookingsByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedBookingsByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Bookings by Customer"));
  setListLabel(tr("Bookings"));
  setReportName("TimePhasedBookingsByCustomer");
  setMetaSQLOptions("timePhasedBookings", "detail");
  setUseAltId(true);

  _customerType->setType(ParameterGroup::CustomerType);

  list()->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft,  true,  "cust_number" );
  list()->addColumn(tr("Customer"), 180,          Qt::AlignLeft,  true,  "cust_name" );
}

void dspTimePhasedBookingsByCustomer::languageChange()
{
  displayTimePhased::languageChange();
  retranslateUi(this);
}

void dspTimePhasedBookingsByCustomer::sViewBookings()
{
  if (_column > 1)
  {
    ParameterList params;
    params.append("cust_id", list()->id());
    params.append("startDate", _columnDates[_column - 2].startDate);
    params.append("endDate", _columnDates[_column - 2].endDate);
    params.append("run");

    dspBookingsByCustomer *newdlg = new dspBookingsByCustomer();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedBookingsByCustomer::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    menuItem = pMenu->addAction(tr("View Bookings..."), this, SLOT(sViewBookings()));
}

bool dspTimePhasedBookingsByCustomer::setParamsTP(ParameterList & params)
{
  params.append("byCust");

  _customerType->appendValue(params);

   return true;
}
