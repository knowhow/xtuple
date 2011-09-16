/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBookingsByProductCategory.h"

#include <QMessageBox>
#include <QVariant>

#include "xtreewidget.h"

dspBookingsByProductCategory::dspBookingsByProductCategory(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBookingsByProductCategory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Bookings by Product Category"));
  setListLabel(tr("Bookings"));
  setReportName("BookingsByProductCategory");
  setMetaSQLOptions("salesOrderItems", "detail");

  _productCategory->setType(ParameterGroup::ProductCategory);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("S/O #"),            _orderColumn,    Qt::AlignLeft,   true,  "coitem_linenumber"   );
  list()->addColumn(tr("Ord. Date"),        _dateColumn,     Qt::AlignCenter, true,  "cohead_orderdate" );
  list()->addColumn(tr("Cust. #"),          _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Customer/Item Number"),      _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Ordered"),          _qtyColumn,      Qt::AlignRight,  true,  "coitem_qtyord"  );
  list()->addColumn(tr("Unit Price"),       _priceColumn,    Qt::AlignRight,  true,  "coitem_price"  );
  list()->addColumn(tr("Ext. Price"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );
  list()->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  list()->addColumn(tr("Base Unit Price"),  _priceColumn,    Qt::AlignRight,  true,  "baseunitprice" );
  list()->addColumn(tr("Base Ext. Price"),  _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
}

void dspBookingsByProductCategory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspBookingsByProductCategory::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    _productCategory->setId(param.toInt());

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    _productCategory->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspBookingsByProductCategory::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid() && isVisible())
  {
    QMessageBox::warning(this, tr("Enter Start Date"),
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
  params.append("orderByOrderdate");

  return true;
}

