/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBookingsBySalesRep.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspBookingsBySalesRep::dspBookingsBySalesRep(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBookingsBySalesRep", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Bookings by Sales Rep."));
  setListLabel(tr("Bookings"));
  setReportName("BookingsBySalesRep");
  setMetaSQLOptions("salesOrderItems", "detail");

  _salesrep->setType(XComboBox::SalesRepsActive);
  _productCategory->setType(ParameterGroup::ProductCategory);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("S/O #"),            _orderColumn,    Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Ord. Date"),        _dateColumn,     Qt::AlignCenter, true,  "cohead_orderdate" );
  list()->addColumn(tr("Cust. #"),          _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Customer"),         -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Item Number"),      _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Ordered"),          _qtyColumn,      Qt::AlignRight,  true,  "coitem_qtyord"  );
  list()->addColumn(tr("Unit Price"),       _priceColumn,    Qt::AlignRight,  true,  "coitem_price"  );
  list()->addColumn(tr("Ext. Price"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );
  list()->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  list()->addColumn(tr("Base Unit Price"),  _priceColumn,    Qt::AlignRight,  true,  "baseunitprice" );
  list()->addColumn(tr("Base Ext. Price"),  _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
}

void dspBookingsBySalesRep::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspBookingsBySalesRep::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid() &&isVisible())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid() && isVisible())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _dates->appendValue(params);
  params.append("salesrep_id", _salesrep->id());
  params.append("orderByOrderdate");

  return true;
}

