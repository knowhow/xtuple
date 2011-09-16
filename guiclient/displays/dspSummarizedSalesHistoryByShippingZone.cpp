/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesHistoryByShippingZone.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspSummarizedSalesHistoryByShippingZone::dspSummarizedSalesHistoryByShippingZone(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedSalesHistoryByShippingZone", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Sales History by Shipping Zone by Item"));
  setListLabel(tr("Sales History"));
  setReportName("SummarizedSalesHistoryByShippingZone");
  setMetaSQLOptions("summarizedSalesHistory", "detail");
  setUseAltId(true);

  connect(_selectedShippingZone, SIGNAL(toggled(bool)), _shipZone, SLOT(setEnabled(bool)));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _shipZone->populate( "SELECT shipzone_id, (shipzone_name || '-' || shipzone_descrip) "
                       "FROM shipzone "
                       "ORDER BY shipzone_name;" );

  list()->addColumn(tr("Zone"),        _itemColumn,     Qt::AlignLeft,   true,  "shipzone_name"   );
  list()->addColumn(tr("Customer"),    200,             Qt::AlignLeft,   true,  "customer"   );
  list()->addColumn(tr("Item Number"), _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "itemdescription"   );
  list()->addColumn(tr("Shipped"),     _qtyColumn,      Qt::AlignRight,  true,  "totalunits"  );
  list()->addColumn(tr("Total Sales"), _bigMoneyColumn, Qt::AlignRight,  true,  "totalsales"  );
}

void dspSummarizedSalesHistoryByShippingZone::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSummarizedSalesHistoryByShippingZone::setParams(ParameterList & params)
{
  if (! _dates->allValid())
  {
    QMessageBox::warning(this, tr("Enter Dates"),
                         tr("Enter valid date(s)."));
    _dates->setFocus();
    return false;
  }

  if (_selectedShippingZone->isChecked())
    params.append("shipzone_id", _shipZone->id());

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  params.append("byShippingZone");

  return true;
}

