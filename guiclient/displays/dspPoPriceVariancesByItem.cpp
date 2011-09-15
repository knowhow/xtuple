/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoPriceVariancesByItem.h"

#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"

dspPoPriceVariancesByItem::dspPoPriceVariancesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoPriceVariancesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Price Variances by Item"));
  setListLabel(tr("Price Variances"));
  setReportName("PurchasePriceVariancesByItem");
  setMetaSQLOptions("poPriceVariances", "detail");

  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  list()->addColumn(tr("P/O #"),              _orderColumn,    Qt::AlignRight,  true,  "porecv_ponumber"  );
  list()->addColumn(tr("Dist. Date"),         _dateColumn,     Qt::AlignCenter, true,  "distdate" );
  list()->addColumn(tr("Recv. Date"),         _dateColumn,     Qt::AlignCenter, false, "receivedate" );
  list()->addColumn(tr("Vendor Number"),      _itemColumn,     Qt::AlignLeft,   true,  "vend_number"   );
  list()->addColumn(tr("Vendor Name"),        -1,              Qt::AlignLeft,   true,  "vend_name"   );
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

void dspPoPriceVariancesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoPriceVariancesByItem::setParams(ParameterList &pParams)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  if (_notZero->isChecked())
    pParams.append("notZero");

  pParams.append("item_id", _item->id());
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

