/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByCustomerByItem.h"

#include <QVariant>
#include <QMessageBox>

dspSummarizedSalesByCustomerByItem::dspSummarizedSalesByCustomerByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedSalesByCustomerByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Sales by Customer by Item"));
  setListLabel(tr("Sales History"));
  setReportName("SummarizedSalesHistoryByCustomerByItem");
  setMetaSQLOptions("summarizedSalesHistory", "detail");

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
  list()->setAltDragString("itemid=");

  _cust->setFocus();
}

void dspSummarizedSalesByCustomerByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSummarizedSalesByCustomerByItem::setParams(ParameterList & params)
{
  if (!_cust->isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Select Customer"),
                            tr("Please select Customer.") );
      _cust->setFocus();
    }
    return false;
  }

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

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  params.append("cust_id", _cust->id());

  params.append("byCustomerByItem"); // metasql only?

  return true;
}

