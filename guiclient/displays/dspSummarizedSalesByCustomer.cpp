/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspSalesHistoryByCustomer.h"

dspSummarizedSalesByCustomer::dspSummarizedSalesByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedSalesByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Sales History by Customer"));
  setListLabel(tr("Sales History"));
  setReportName("SummarizedSalesHistoryByCustomer");
  setMetaSQLOptions("summarizedSalesHistory", "detail");

  _productCategory->setType(ParameterGroup::ProductCategory);
  _currency->setType(ParameterGroup::Currency);

  list()->addColumn(tr("Customer"),    -1,               Qt::AlignLeft,   true,  "customer"   );
  list()->addColumn(tr("First Sale"),  _dateColumn,      Qt::AlignCenter, true,  "firstsale" );
  list()->addColumn(tr("Last Sale"),   _dateColumn,      Qt::AlignCenter, true,  "lastsale" );
  list()->addColumn(tr("Total Qty."),  _qtyColumn,       Qt::AlignRight,  true,  "qtyshipped"  );
  list()->addColumn(tr("Total Sales"), _bigMoneyColumn,  Qt::AlignRight,  true,  "extprice"  );
  list()->addColumn(tr("Currency"),    _currencyColumn,  Qt::AlignLeft,   true,  "currAbbr"  );

  if (omfgThis->singleCurrency())
    list()->hideColumn(5);
}

void dspSummarizedSalesByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSummarizedSalesByCustomer::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewDetail()));
}

void dspSummarizedSalesByCustomer::sViewDetail()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _dates->appendValue(params);
  params.append("cust_id", list()->id());
  params.append("run");

  dspSalesHistoryByCustomer *newdlg = new dspSalesHistoryByCustomer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspSummarizedSalesByCustomer::setParams(ParameterList & params)
{
  if (!_dates->startDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return false;
  }

  if (_productCategory->isPattern())
  {
    QString pattern = _productCategory->pattern();
    if (pattern.length() == 0)
      return false;
  }

  if (_currency->isPattern())
  {
    QString pattern = _currency->pattern();
    if (pattern.length() == 0)
      return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);

  if (_currency->isSelected())
    params.append("curr_id", _currency->id());
  else if (_currency->isPattern())
    params.append("currConcat_pattern", _currency->pattern());

  params.append("byCustomer"); // metasql only?

  return true;
}

