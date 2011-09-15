/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspFreightPricesByCustomer.h"
#include "xtreewidget.h"

#include <QMessageBox>

dspFreightPricesByCustomer::dspFreightPricesByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspFreightPricesByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Freight Prices by Customer"));
  setListLabel(tr("Freight Prices"));
  setReportName("FreightPricesByCustomer");
  setMetaSQLOptions("freightPrices", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Schedule"),      _itemColumn,     Qt::AlignLeft,   true,  "ipshead_name"  );
  list()->addColumn(tr("Source"),        _itemColumn,     Qt::AlignLeft,   true,  "source"  );
  list()->addColumn(tr("Qty. Break"),    _qtyColumn,      Qt::AlignRight,  true,  "ipsfreight_qtybreak" );
  list()->addColumn(tr("Price"),         _priceColumn,    Qt::AlignRight,  true,  "ipsfreight_price" );
  list()->addColumn(tr("Method"),        _itemColumn,     Qt::AlignLeft,   true,  "method"  );
  list()->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,   true,  "currConcat");
  list()->addColumn(tr("From"),          _itemColumn,     Qt::AlignLeft,   true,  "warehous_code"  );
  list()->addColumn(tr("To"),            _itemColumn,     Qt::AlignLeft,   true,  "shipzone_name"  );
  list()->addColumn(tr("Freight Class"), _itemColumn,     Qt::AlignLeft,   true,  "freightclass_code"  );
  list()->addColumn(tr("Ship Via"),      _itemColumn,     Qt::AlignLeft,   true,  "ipsfreight_shipvia"  );

}

void dspFreightPricesByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspFreightPricesByCustomer::setParams(ParameterList &params)
{
  if(!_cust->isValid())
  {
    QMessageBox::warning(this, tr("Customer Required"),
      tr("You must specify a valid Customer."));
    return false;
  }

  params.append("byCust");

  params.append("na", tr("N/A"));
  params.append("any", tr("Any"));
  params.append("flatrate", tr("Flat Rate"));
  params.append("peruom", tr("Per UOM"));
  params.append("customer", tr("Customer"));
  params.append("custType", tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));

  if (_cust->isValid())
    params.append("cust_id", _cust->id());

  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");

  return true;
}
