/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesHistoryBySalesrep.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "salesHistoryInformation.h"

dspSalesHistoryBySalesrep::dspSalesHistoryBySalesrep(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesHistoryBySalesrep", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales History by Sales Rep."));
  setListLabel(tr("Sales History"));
  setReportName("SalesHistoryBySalesRep");
  setMetaSQLOptions("salesHistory", "detail");

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _salesrep->setType(XComboBox::SalesRepsActive);
  _productCategory->setType(ParameterGroup::ProductCategory);

  list()->addColumn(tr("Customer"),            -1,              Qt::AlignLeft,   true,  "cust_name"  );
  list()->addColumn(tr("Doc. #"),              _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"  );
  list()->addColumn(tr("Invoice #"),           _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"  );
  list()->addColumn(tr("Ord. Date"),           _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate");
  list()->addColumn(tr("Invc. Date"),          _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate");
  list()->addColumn(tr("Item Number"),         _itemColumn,     Qt::AlignLeft,   true,  "item_number"  );
  list()->addColumn(tr("Shipped"),             _qtyColumn,      Qt::AlignRight,  true,  "cohist_qtyshipped" );
  if (_privileges->check("ViewCustomerPrices"))
  {
    list()->addColumn(tr("Unit Price"),        _priceColumn,    Qt::AlignRight,  true,  "cohist_unitprice" );
    list()->addColumn(tr("Ext. Price"),        _bigMoneyColumn, Qt::AlignRight,  true,  "extprice" );
    list()->addColumn(tr("Currency"),          _currencyColumn, Qt::AlignRight,  true,  "currAbbr" );
    list()->addColumn(tr("Base Unit Price"),   _bigMoneyColumn, Qt::AlignRight,  true,  "baseunitprice" );
    list()->addColumn(tr("Base Ext. Price"),   _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
  }
  if (_privileges->check("ViewCosts"))
  {
    list()->addColumn(tr("Unit Cost"),         _costColumn,     Qt::AlignRight,  true,  "cohist_unitcost" );
    list()->addColumn(tr("Ext. Cost"),         _bigMoneyColumn, Qt::AlignRight,  true,  "extcost" );
  }

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();

  _salesrep->setFocus();
}

void dspSalesHistoryBySalesrep::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSalesHistoryBySalesrep::set(const ParameterList &pParams)
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
  else
    _warehouse->setAll();

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

void dspSalesHistoryBySalesrep::sHandleParams()
{
  if (_showPrices->isChecked())
  {
    list()->showColumn(list()->column("cohist_unitprice"));
    list()->showColumn(list()->column("extprice"));
    list()->showColumn(list()->column("currAbbr"));
    list()->showColumn(list()->column("baseunitprice"));
    list()->showColumn(list()->column("baseextprice"));
  }
  else
  {
    list()->hideColumn(list()->column("cohist_unitprice"));
    list()->hideColumn(list()->column("extprice"));
    list()->hideColumn(list()->column("currAbbr"));
    list()->hideColumn(list()->column("baseunitprice"));
    list()->hideColumn(list()->column("baseextprice"));
  }

  if (_showCosts->isChecked())
  {
    list()->showColumn(list()->column("cohist_unitprice"));
    list()->showColumn(list()->column("extcost"));
  }
  else
  {
    list()->hideColumn(list()->column("cohist_unitprice"));
    list()->hideColumn(list()->column("extcost"));
  }
}

void dspSalesHistoryBySalesrep::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("EditSalesHistory"))
    menuItem->setEnabled(false);

  pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void dspSalesHistoryBySalesrep::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", list()->id());

  salesHistoryInformation newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspSalesHistoryBySalesrep::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", list()->id());

  salesHistoryInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

bool dspSalesHistoryBySalesrep::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  params.append("salesrep_id", _salesrep->id());
  params.append("orderByInvcdateItem"); // metasql only?

  params.append("includeFormatted"); // report only?

  if (_showCosts->isChecked())
    params.append("showCosts"); // report only?
  if (_showPrices->isChecked())
    params.append("showPrices"); // report only?

  return true;
}

