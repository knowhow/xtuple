/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByCustomerTypeByItem.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspSummarizedSalesByCustomerTypeByItem::dspSummarizedSalesByCustomerTypeByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedSalesByCustomerTypeByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Sales by Customer Type by Item"));
  setListLabel(tr("Sales History"));
  setReportName("SummarizedSalesHistoryByCustomerTypeByItem");
  setMetaSQLOptions("summarizedSalesHistory", "detail");

  _customerType->setType(ParameterGroup::CustomerType);

  list()->addColumn(tr("Item Number"),     _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),     -1 ,             Qt::AlignLeft,   true,  "itemdescription"   );
  list()->addColumn(tr("Site"),            _whsColumn,      Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Min. Price"),      _priceColumn,    Qt::AlignRight,  true,  "minprice"  );
  list()->addColumn(tr("Max. Price"),      _priceColumn,    Qt::AlignRight,  true,  "maxprice"  );
  list()->addColumn(tr("Avg. Price"),      _priceColumn,    Qt::AlignRight,  true,  "avgprice"  );
  list()->addColumn(tr("Wt. Avg. Price"),  _priceColumn,    Qt::AlignRight,  true,  "wtavgprice"  );
  list()->addColumn(tr("Total Units"),     _qtyColumn,      Qt::AlignRight,  true,  "totalunits"  );
  list()->addColumn(tr("Total Sales"),     _bigMoneyColumn, Qt::AlignRight,  true,  "totalsales"  );
  list()->setDragString("itemsiteid=");
}

void dspSummarizedSalesByCustomerTypeByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSummarizedSalesByCustomerTypeByItem::setParams(ParameterList & params)
{
  if (!_dates->startDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return false;
  }

  if (_customerType->isPattern())
  {
    QString pattern = _customerType->pattern();
    if (pattern.length() == 0)
      return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _customerType->appendValue(params);

  params.append("byCustomerTypeByItem"); // metasql only?

  return true;
}

