/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesHistory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "parameterwidget.h"
#include "salesHistoryInformation.h"

dspSalesHistory::dspSalesHistory(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesHistory", fl)
{
  setWindowTitle(tr("Sales History"));
  setReportName("SalesHistory");
  setMetaSQLOptions("salesHistory", "detail");
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Invoice Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Invoice End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Ship Start Date"), "shipStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Ship End Date"),   "shipEndDate",   ParameterWidget::Date);
  parameterWidget()->append(tr("Customer"),   "cust_id",   ParameterWidget::Customer);
  parameterWidget()->append(tr("Customer Ship-to"),   "shipto_id",   ParameterWidget::Shipto);
  parameterWidget()->appendComboBox(tr("Customer Group"), "custgrp_id", XComboBox::CustomerGroups);
  parameterWidget()->append(tr("Customer Group Pattern"), "custgrp_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Sales Order"), "cohead_id", ParameterWidget::SalesOrder);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesReps);
  parameterWidget()->appendComboBox(tr("Shipping Zones"), "shipzone_id", XComboBox::ShippingZones);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

  list()->addColumn(tr("Customer"),            -1,              Qt::AlignLeft,   true,  "cust_name"   );
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
    list()->addColumn(tr("Unit Cost"),         _costColumn,     Qt::AlignRight,  true,  "cohist_unitcost" );
    list()->addColumn(tr("Ext. Cost"),         _bigMoneyColumn, Qt::AlignRight,  true,  "extcost" );
  }
}

enum SetResponse dspSalesHistory::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  parameterWidget()->setSavedFilters();

  param = pParams.value("cohead_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Sales Order"), param.toInt());

  param = pParams.value("cust_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer"), param.toInt());

  param = pParams.value("custtype_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer Type"), param.toInt());

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Customer Type Pattern"), param.toString());

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param.toInt());

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Product Category"), param.toInt());

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Product Category Pattern"), param.toString());

  param = pParams.value("salesrep_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Sales Rep."), param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Invoice Start Date"), param.toDate());
  else
    parameterWidget()->setDefault(tr("Invoice Start Date"), QVariant());

  param = pParams.value("endDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Invoice End Date"), param.toDate());
  else
    parameterWidget()->setDefault(tr("Invoice End Date"), QVariant());

  param = pParams.value("shipStartDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Ship Start Date"), param.toDate());
  else
    parameterWidget()->setDefault(tr("Ship Start Date"), QVariant());

  param = pParams.value("shipEndDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Ship End Date"), param.toDate());
  else
    parameterWidget()->setDefault(tr("Ship End Date"), QVariant());

  param = pParams.value("shipzone_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Shipping Zones"), param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Site"), param.toInt());

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspSalesHistory::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;
  if (_privileges->check("ViewCustomerPrices"))
    params.append("showPrices");

  return true;
}

void dspSalesHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("EditSalesHistory"))
    menuItem->setEnabled(false);

  pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void dspSalesHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", list()->id());

  salesHistoryInformation newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspSalesHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", list()->id());

  salesHistoryInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

