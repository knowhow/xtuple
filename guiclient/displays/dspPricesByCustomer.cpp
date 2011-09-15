/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByCustomer.h"

#include <QMessageBox>

#include "guiclient.h"

#define CURR_COL	7
#define COST_COL	8
#define MARGIN_COL	9

dspPricesByCustomer::dspPricesByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPricesByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Prices by Customer"));
  setListLabel(tr("Prices"));
  setReportName("PricesByCustomer");
  setMetaSQLOptions("prices", "detail");
  setUseAltId(true);

  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  list()->addColumn(tr("Schedule"),    _itemColumn,     Qt::AlignLeft,   true,  "schedulename"  );
  list()->addColumn(tr("Source"),      _itemColumn,     Qt::AlignLeft,   true,  "type"  );
  list()->addColumn(tr("Item Number"), _itemColumn,     Qt::AlignLeft,   true,  "itemnumber"  );
  list()->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "itemdescrip"  );
  list()->addColumn(tr("Price UOM"),   _uomColumn,      Qt::AlignCenter, true,  "priceuom");
  list()->addColumn(tr("Qty. Break"),  _qtyColumn,      Qt::AlignRight,  true,  "f_qtybreak" );
  list()->addColumn(tr("Price"),       _priceColumn,    Qt::AlignRight,  true,  "price" );
  list()->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currConcat");
  list()->addColumn(tr("Ext. Cost"),   _costColumn,     Qt::AlignRight,  true,  "f_cost" );
  list()->addColumn(tr("Mar. %"),      _prcntColumn,    Qt::AlignRight,  true,  "f_margin" );

  if (omfgThis->singleCurrency())
    list()->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());
}

void dspPricesByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspPricesByCustomer::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
  {
    list()->showColumn(COST_COL);
    list()->showColumn(MARGIN_COL);
  }
  else
  {
    list()->hideColumn(COST_COL);
    list()->hideColumn(MARGIN_COL);
  }
  _costsGroup->setEnabled(_showCosts->isChecked());
}

bool dspPricesByCustomer::setParams(ParameterList & params)
{
  if(!_cust->isValid())
  {
    QMessageBox::warning(this, tr("Customer Required"),
      tr("You must specify a Customer."));
    return false;
  }

  params.append("na", tr("N/A"));
  params.append("costna", tr("?????"));
  params.append("customer", tr("Customer"));
  params.append("custType", tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));
  params.append("listPrice", tr("List Price"));
  params.append("cust_id", _cust->id());
  params.append("byCustomer");

  if (_showCosts->isChecked())
  {
    params.append("showCosts");
    if (_useStandardCosts->isChecked())
    {
      params.append("useStandardCosts"); // metasql only?
      params.append("standardCosts"); // report only?
    }
    else if (_useActualCosts->isChecked())
    {
      params.append("useActualCosts"); // metasql only?
      params.append("actualCosts"); // report only?
    }
  }

  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");

  return true;
}
