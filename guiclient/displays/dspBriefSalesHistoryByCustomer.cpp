/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistoryByCustomer.h"

#include <QVariant>
#include <QMessageBox>

#include "xtreewidget.h"

dspBriefSalesHistoryByCustomer::dspBriefSalesHistoryByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBriefSalesHistoryByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Brief Sales History by Customer"));
  setListLabel(tr("Sales History"));
  setReportName("BriefSalesHistoryByCustomer");
  setMetaSQLOptions("briefSalesHistory", "detail");

  _productCategory->setType(ParameterGroup::ProductCategory);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Doc. #"),     _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Cust. P/O #"), -1,             Qt::AlignLeft,   true,  "cohist_ponumber"   );
  list()->addColumn(tr("Invoice #"),  _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  list()->addColumn(tr("Ord. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  list()->addColumn(tr("Invc. Date"), _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn(tr("Total"),      _bigMoneyColumn, Qt::AlignRight,  true,  "extended"  );
}

void dspBriefSalesHistoryByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspBriefSalesHistoryByCustomer::setParams(ParameterList &params)
{
  if (!_cust->isValid())
  {
    if(isVisible())
    {
      QMessageBox::warning( this, tr("Enter Customer Number"),
                           tr("Please enter a valid Customer Number.") );
      _cust->setFocus();
    }
    return false;
  }

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

  params.append("cust_id", _cust->id());
  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);

  return true;
}
