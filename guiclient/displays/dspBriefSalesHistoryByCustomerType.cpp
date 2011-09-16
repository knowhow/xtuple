/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistoryByCustomerType.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspBriefSalesHistoryByCustomerType::dspBriefSalesHistoryByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspBriefSalesHistoryByCustomerType", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Brief Sales History by Customer Type"));
  setListLabel(tr("Sales History"));
  setReportName("BriefSalesHistoryByCustomerType");
  setMetaSQLOptions("briefSalesHistory", "detail");

  _customerType->setType(ParameterGroup::CustomerType);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Cust. Type"),  _orderColumn,    Qt::AlignLeft,   true,  "custtype_code"   );
  list()->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Doc. #"),      _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Invoice #"),   _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  list()->addColumn(tr("Ord. Date"),   _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  list()->addColumn(tr("Invc. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn(tr("Total"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extended"  );
}

void dspBriefSalesHistoryByCustomerType::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspBriefSalesHistoryByCustomerType::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid())
  {
    if(isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return false;
  }
  else if (!_dates->endDate().isValid())
  {
    if(isVisible())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return false;
  }
  else
    _dates->appendValue(params);

  _warehouse->appendValue(params);
  _customerType->appendValue(params);

  return true;
}
