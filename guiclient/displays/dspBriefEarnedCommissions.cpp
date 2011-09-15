/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefEarnedCommissions.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspBriefEarnedCommissions::dspBriefEarnedCommissions(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBriefEarnedCommissions", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Brief Earned Commissions"));
  setListLabel(tr("Earned Commissions"));
  setReportName("BriefEarnedCommissions");
  setMetaSQLOptions("briefEarnedCommission", "detail");

  connect(_selectedSalesrep, SIGNAL(toggled(bool)), _salesrep, SLOT(setEnabled(bool)));

  _salesrep->setType(XComboBox::SalesReps);

  list()->addColumn(tr("#"),               _seqColumn,      Qt::AlignCenter, true,  "salesrep_number" );
  list()->addColumn(tr("Sales Rep."),      _itemColumn,     Qt::AlignLeft,   true,  "salesrep_name"   );
  list()->addColumn(tr("Cust. #"),         _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Customer"),        -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("S/O #"),           _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Invoice #"),       _orderColumn,    Qt::AlignLeft,   true,  "cohist_invcnumber"   );
  list()->addColumn(tr("Invc. Date"),      _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn(tr("Ext. Price"),      _moneyColumn,    Qt::AlignRight,  true,  "sumextprice"  );
  list()->addColumn(tr("Commission"),      _moneyColumn,    Qt::AlignRight,  true,  "sumcommission"  );
  list()->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  list()->addColumn(tr("Base Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "sumbaseextprice"  );
  list()->addColumn(tr("Base Commission"), _bigMoneyColumn, Qt::AlignRight,  true,  "sumbasecommission"  );
}

void dspBriefEarnedCommissions::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspBriefEarnedCommissions::setParams(ParameterList &params)
{
  if (!_dates->allValid())
  {
    if (!_dates->startDate().isValid() && isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
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
  }
  else
    _dates->appendValue(params);
  if (_includeMisc->isChecked())
    params.append("includeMisc");

  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());

  return true;
}

