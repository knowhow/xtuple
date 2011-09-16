/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistoryBySalesRep.h"

#include <QMessageBox>
#include <QVariant>

#include "xtreewidget.h"

dspBriefSalesHistoryBySalesRep::dspBriefSalesHistoryBySalesRep(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspBriefSalesHistoryBySalesRep", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Brief Sales History by Sales Rep."));
  setListLabel(tr("Sales History"));
  setReportName("SalesHistoryBySalesRep");
  setMetaSQLOptions("briefSalesHistory", "detail");

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _salesrep->setType(XComboBox::SalesRepsActive);
  _productCategory->setType(ParameterGroup::ProductCategory);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Doc. #"),      _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Invoice #"),   _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  list()->addColumn(tr("Ord. Date"),   _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  list()->addColumn(tr("Invc. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn( tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "extended"  );
  list()->addColumn( tr("Ext. Cost"),  _bigMoneyColumn, Qt::AlignRight,  true,  "extcost"  );

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();

  _salesrep->setFocus();
}

void dspBriefSalesHistoryBySalesRep::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspBriefSalesHistoryBySalesRep::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("salesrep_id", &valid);
  if (valid)
    _salesrep->setId(param.toInt());

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    _productCategory->setId(param.toInt());

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    _productCategory->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspBriefSalesHistoryBySalesRep::sHandleParams()
{
  if (_showPrices->isChecked())
    list()->showColumn(5);
  else
    list()->hideColumn(5);

  if (_showCosts->isChecked())
    list()->showColumn(6);
  else
    list()->hideColumn(6);
}

bool dspBriefSalesHistoryBySalesRep::setParams(ParameterList &params)
{
  if (isVisible())
  {
    if (!_dates->startDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return false;
    }
    else if (!_dates->endDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return false;
    }
    else
      _dates->appendValue(params);
  }
  
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  params.append("salesrep_id", _salesrep->id());

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_showPrices->isChecked())
    params.append("showPrices");

  return true;
}
