/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspEarnedCommissions.h"

#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"
#include "xtreewidget.h"

dspEarnedCommissions::dspEarnedCommissions(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspEarnedCommissions", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Earned Commissions"));
  setListLabel(tr("Earned Commissions"));
  setReportName("EarnedCommissions");
  setMetaSQLOptions("salesHistory", "detail");

  connect(_selectedSalesrep, SIGNAL(toggled(bool)), _salesrep, SLOT(setEnabled(bool)));

  _salesrep->setType(XComboBox::SalesReps);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Sales Rep."),      100,             Qt::AlignLeft,   true,  "salesrep_name"   );
  list()->addColumn(tr("S/O #"),           _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Cust. #"),         _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Ship-To"),         -1,              Qt::AlignLeft,   true,  "cohist_shiptoname"   );
  list()->addColumn(tr("Invc. Date"),      _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn(tr("Item Number"),     _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Qty."),            _qtyColumn,      Qt::AlignRight,  true,  "cohist_qtyshipped"  );
  list()->addColumn(tr("Ext. Price"),      _moneyColumn,    Qt::AlignRight,  true,  "extprice"  );
  list()->addColumn(tr("Commission"),      _moneyColumn,    Qt::AlignRight,  true,  "cohist_commission"  );
  list()->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  list()->addColumn(tr("Base Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice"  );
  list()->addColumn(tr("Base Commission"), _bigMoneyColumn, Qt::AlignRight,  true,  "basecommission"  );
  list()->addColumn(tr("Paid"),            _ynColumn,       Qt::AlignCenter, true,  "cohist_commissionpaid" );
}

void dspEarnedCommissions::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspEarnedCommissions::setParams(ParameterList &params)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start and End Date"),
                          tr("You must enter a valid Start and End Date for this report.") );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);
  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());
  if (_includeMisc->isChecked())
    params.append("includeMisc");
  params.append("includeFormatted"); // report only?
  params.append("orderBySalesRepInvcdate"); // metasql only?

  return true;
}

