/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspFreightPricesByCustomerType.h"
#include "xtreewidget.h"

dspFreightPricesByCustomerType::dspFreightPricesByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspFreightPricesByCustomerType", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Freight Prices by Customer Type"));
  setListLabel(tr("Freight Prices"));
  setReportName("FreightPricesByCustomerType");
  setMetaSQLOptions("freightPrices", "detail");
  setUseAltId(true);

  _custtype->setType(XComboBox::CustomerTypes);

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

void dspFreightPricesByCustomerType::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspFreightPricesByCustomerType::setParams(ParameterList &params)
{
  params.append("byCustType");

  params.append("na", tr("N/A"));
  params.append("any", tr("Any"));
  params.append("flatrate", tr("Flat Rate"));
  params.append("peruom", tr("Per UOM"));
  params.append("custType", tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));

  if (_custtype->isValid())
    params.append("custtype_id", _custtype->id());

  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");

  return true;
}
