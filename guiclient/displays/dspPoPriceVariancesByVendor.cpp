/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoPriceVariancesByVendor.h"

#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"

dspPoPriceVariancesByVendor::dspPoPriceVariancesByVendor(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoPriceVariancesByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Price Variances by Vendor"));
  setListLabel(tr("Price Variances"));
  setReportName("PurchasePriceVariancesByVendor");
  setMetaSQLOptions("poPriceVariances", "detail");

  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));

  _agent->populate( "SELECT usesysid, usename "
                    "FROM pg_user, usr "
                    "WHERE ( (usename=usr_username)"
                    " AND (usr_active)"
                    " AND (usr_agent) ) "
                    "ORDER BY usename;" );
  
  list()->addColumn(tr("P/O #"),              _orderColumn,    Qt::AlignRight,  true,  "porecv_ponumber"  );
  list()->addColumn(tr("Dist. Date"),         _dateColumn,     Qt::AlignCenter, true,  "distdate" );
  list()->addColumn(tr("Recv. Date"),         _dateColumn,     Qt::AlignCenter, false, "receivedate" );
  list()->addColumn(tr("Vendor Number"),      _itemColumn,     Qt::AlignLeft,   false,  "vend_number"   );
  list()->addColumn(tr("Vendor Name"),        -1,              Qt::AlignLeft,   false,  "vend_name"   );
  list()->addColumn(tr("Item Number"),        _itemColumn,     Qt::AlignLeft,   true,  "itemnumber"   );
  list()->addColumn(tr("Description"),        -1,              Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Qty."),               _qtyColumn,      Qt::AlignRight,  true,  "porecv_qty"  );
  list()->addColumn(tr("Purch. Cost"),        _priceColumn,    Qt::AlignRight,  false, "porecv_purchcost"  );
  if (!omfgThis->singleCurrency())
    list()->addColumn(tr("Purch. Curr."),       _priceColumn,    Qt::AlignRight,  false, "poCurrAbbr"  );
  list()->addColumn(tr("Rcpt. Cost"),         _priceColumn,    Qt::AlignRight,  false, "porecv_recvcost"  );
  list()->addColumn(tr("Received"),           _moneyColumn,    Qt::AlignRight,  true,  "porecv_value"  );
  list()->addColumn(tr("Vouch. Cost"),        _priceColumn,    Qt::AlignRight,  false, "vouchercost"  );
  list()->addColumn(tr("Vouchered"),          _moneyColumn,    Qt::AlignRight,  true,  "voucher_value"  );
  list()->addColumn(tr("Variance"),           _moneyColumn,    Qt::AlignRight,  true,  "variance"  );
  if (!omfgThis->singleCurrency())
    list()->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignRight,  true,  "currAbbr"  );
  list()->addColumn(tr("%"),                  _prcntColumn,    Qt::AlignRight,  true,  "varprcnt"  );

  _currencyGroup->setHidden(omfgThis->singleCurrency());
}

void dspPoPriceVariancesByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoPriceVariancesByVendor::setParams(ParameterList &pParams)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  if (_notZero->isChecked())
    pParams.append("notZero");

  _vendorGroup->appendValue(pParams);
  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_baseCurr->isChecked())
    pParams.append("baseCurr");

  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  pParams.append("includeFormatted"); // report only?

  return true;
}

