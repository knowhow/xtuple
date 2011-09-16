/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesHistoryByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "salesHistoryInformation.h"
#include "dspInvoiceInformation.h"

dspSalesHistoryByCustomer::dspSalesHistoryByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesHistoryByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales History by Customer"));
  setListLabel(tr("Sales History"));
  setReportName("SalesHistoryByCustomer");
  setMetaSQLOptions("salesHistory", "detail");

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _productCategory->setType(ParameterGroup::ProductCategory);

  list()->addColumn(tr("Doc. #"),              _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Invoice #"),           _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  list()->addColumn(tr("Ord. Date"),           _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  list()->addColumn(tr("Invc. Date"),          _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn(tr("Item Number"),         _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),         -1,              Qt::AlignLeft,   true,  "itemdescription"   );
  list()->addColumn(tr("Shipped"),             _qtyColumn,      Qt::AlignRight,  true,  "cohist_qtyshipped"  );
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
    list()->addColumn( tr("Unit Cost"),  _costColumn,           Qt::AlignRight,  true,  "cohist_unitcost" );
    list()->addColumn( tr("Ext. Cost"),  _bigMoneyColumn,       Qt::AlignRight,  true,  "extcost" );
  }

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();

  _cust->setFocus();
}

void dspSalesHistoryByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSalesHistoryByCustomer::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    _cust->setReadOnly(true);
  }

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

void dspSalesHistoryByCustomer::sHandleParams()
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
    list()->showColumn(list()->column("cohist_unitcost"));
    list()->showColumn(list()->column("extcost"));
  }
  else
  {
    list()->hideColumn(list()->column("cohist_unitcost"));
    list()->hideColumn(list()->column("extcost"));
  }
}

void dspSalesHistoryByCustomer::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;
  XTreeWidgetItem * item = (XTreeWidgetItem*)list()->currentItem();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("EditSalesHistory"));

  pMenu->addAction(tr("View..."), this, SLOT(sView()));

  if (item->rawValue("invoicenumber").toString().length() > 0)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Invoice Information..."), this, SLOT(sInvoiceInformation()));
  }
}

void dspSalesHistoryByCustomer::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", list()->id());

  salesHistoryInformation newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspSalesHistoryByCustomer::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", list()->id());

  salesHistoryInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspSalesHistoryByCustomer::sInvoiceInformation()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)list()->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  params.append("invoiceNumber", item->rawValue("invoicenumber").toString());

  dspInvoiceInformation *newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspSalesHistoryByCustomer::setParams(ParameterList &params)
{
  if (!_cust->isValid())
  {
    QMessageBox::warning( this, tr("Enter Customer Number"),
                          tr("Please enter a valid Customer Number.") );
    _cust->setFocus();
    return false;
  }

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
  params.append("cust_id", _cust->id());
  params.append("orderByInvcdateItem");

  params.append("includeFormatted"); // report only?

  if (_showCosts->isChecked())
    params.append("showCosts"); // report only?
  if (_showPrices->isChecked())
    params.append("showPrices"); // report only?

  return true;
}

