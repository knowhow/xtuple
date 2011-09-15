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
#include "dspSummarizedSales.h"
#include "parameterwidget.h"
#include "xtreewidget.h"
#include "xtsettings.h"

#include <QVariant>
#include <QMessageBox>

dspSummarizedSales::dspSummarizedSales(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedSales", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Sales"));
  setReportName("SummarizedSalesHistory");
  setMetaSQLOptions("summarizedSalesHistory", "detail");
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Invoice Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Invoice End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Ship Start Date"), "shipStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Ship End Date"),   "shipEndDate",   ParameterWidget::Date);
  parameterWidget()->appendComboBox(tr("Currency"), "curr_id", XComboBox::Currencies);
  parameterWidget()->append(tr("Currency Pattern"), "currConcat_patternn", ParameterWidget::Text);
  parameterWidget()->append(tr("Customer"),   "cust_id",   ParameterWidget::Customer);
  parameterWidget()->append(tr("Customer Ship-to"),   "shipto_id",   ParameterWidget::Shipto);
  parameterWidget()->appendComboBox(tr("Customer Group"), "custgrp_id", XComboBox::CustomerGroups);
  parameterWidget()->append(tr("Customer Group Pattern"), "custgrp_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesReps);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

  _units->append(0,tr("Base"));
  _units->append(1,tr("Local"));

  if (!_metrics->boolean("MultiWhs"))
  {
    _site->setForgetful(true);
    _site->setChecked(false);
    _site->setVisible(false);
  }

  if (omfgThis->singleCurrency())
  {
    _unitsLit->hide();
    _units->hide();
  }

  _units->setId(xtsettingsValue("dspSummarizedSales/units").toInt());

  connect(_cust, SIGNAL(toggled(bool)), this, SLOT(sGroupByChanged()));
  connect(_custtype, SIGNAL(toggled(bool)), this, SLOT(sGroupByChanged()));
  connect(_item, SIGNAL(toggled(bool)), this, SLOT(sGroupByChanged()));
  connect(_site, SIGNAL(toggled(bool)), this, SLOT(sGroupByChanged()));
  connect(_shipzone, SIGNAL(toggled(bool)), this, SLOT(sGroupByChanged()));
  connect(_salesrep, SIGNAL(toggled(bool)), this, SLOT(sGroupByChanged()));
  connect(_units, SIGNAL(currentIndexChanged(int)), this, SLOT(sGroupByChanged()));

  sGroupByChanged();
}

void dspSummarizedSales::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSummarizedSales::setParams(ParameterList & params)
{
  if (!_cust->isChecked() &&
      !_custtype->isChecked() &&
      !_salesrep->isChecked() &&
      !_shipzone->isChecked() &&
      !_item->isChecked() &&
      !_site->isChecked())
  {
    QMessageBox::warning( this, tr("Select a Group By"),
                          tr("Please select at least one Group By option.") );
    return false;
  }

  parameterWidget()->appendValue(params);
  params.append("filter", parameterWidget()->filter());

  QList<QVariant> groupLitList;
  QList<QVariant> groupList;
  QList<QVariant> groupDescripList;
  if (_cust->isChecked())
  {
    groupLitList.append(tr("'Customer:'"));
    groupList.append("cust_number");
    groupDescripList.append("cust_name");
    params.append("byCustomer");
  }
  if (_custtype->isChecked())
  {
    groupLitList.append(tr("'Cust. Type:'"));
    groupList.append("custtype_code");
    groupDescripList.append("custtype_descrip");
    params.append("byCustomerType");
  }
  if (_salesrep->isChecked())
  {
    groupLitList.append(tr("'Sales Rep.:'"));
    groupList.append("salesrep_number");
    groupDescripList.append("salesrep_name");
    params.append("bySalesRep");
  }
  if (_shipzone->isChecked())
  {
    groupLitList.append(tr("'Ship Zone:'"));
    groupList.append("COALESCE(shipzone_name,'')");
    groupDescripList.append("COALESCE(shipzone_descrip,'')");
    params.append("byShippingZone");
  }
  if (_item->isChecked())
  {
    groupLitList.append(tr("'Item:'"));
    groupList.append("item_number");
    groupDescripList.append("itemdescription");
    params.append("byItem");
  }
  if (_site->isChecked())
  {
    groupLitList.append(tr("'Site:'"));
    groupList.append("warehous_code");
    groupDescripList.append("warehous_descrip");
    params.append("bySite");
  }
  if (_units->currentIndex())
  {
    params.append("byCurrency");
  }

  params.append("groupLitList", groupLitList);
  params.append("groupList", groupList);
  params.append("groupDescripList", groupDescripList);

  return true;
}

void dspSummarizedSales::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Sales Detail..."), this, SLOT(sViewHistory()));

}

void dspSummarizedSales::sViewHistory()
{
  ParameterList params;
  parameterWidget()->appendValue(params);

  if (_cust->isChecked())
      params.append("cust_id", list()->id("cust_number"));
  if (_custtype->isChecked())
      params.append("custtype_id", list()->id("custtype_code"));
  if (_salesrep->isChecked())
      params.append("salesrep_id", list()->id("salesrep_number"));
  if (_shipzone->isChecked())
      params.append("shipzone_id", list()->id("shipzone_name"));
  if (_item->isChecked())
      params.append("item_id", list()->id("item_number"));
  if (_site->isChecked())
      params.append("warehous_id", list()->id("warehous_code"));
  params.append("run");

  dspSalesHistory *newdlg = new dspSalesHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedSales::sGroupByChanged()
{
  list()->clear();
  list()->setColumnCount(0);

  if (_cust->isChecked())
  {
    list()->addColumn(tr("Customer"),       _itemColumn,      Qt::AlignLeft,   true,  "cust_number"   );
    list()->addColumn(tr("Name"),           -1,               Qt::AlignLeft,   true,  "cust_name" );
  }
  if (_custtype->isChecked())
  {
    list()->addColumn(tr("Customer Type"),  _itemColumn,      Qt::AlignLeft,   true,  "custtype_code"   );
  }
  if (_salesrep->isChecked())
  {
    list()->addColumn(tr("Sales Rep."),     _itemColumn,      Qt::AlignLeft,   true,  "salesrep_number"   );
    list()->addColumn(tr("Name"),           -1,               Qt::AlignLeft,   true,  "salesrep_name" );
  }
  if (_shipzone->isChecked())
    list()->addColumn(tr("Zone"),        _itemColumn,     Qt::AlignLeft,   true,  "shipzone_name"   );
  if (_item->isChecked())
  {
    list()->addColumn(tr("Item"),           _itemColumn,      Qt::AlignLeft,   true,  "item_number"   );
    list()->addColumn(tr("Description"),    -1,               Qt::AlignLeft,   true,  "itemdescription"   );
  }
  if (_site->isChecked())
    list()->addColumn(tr("Site"),         _whsColumn,      Qt::AlignCenter, true,  "warehous_code" );

  list()->addColumn(tr("First Sale"),     _dateColumn,     Qt::AlignCenter, true,  "firstdate" );
  list()->addColumn(tr("Last Sale"),      _dateColumn,     Qt::AlignCenter, true,  "lastdate" );
  list()->addColumn(tr("Min. Price"),     _priceColumn,    Qt::AlignRight,  true,  "minprice"  );
  list()->addColumn(tr("Max. Price"),     _priceColumn,    Qt::AlignRight,  true,  "maxprice"  );
  list()->addColumn(tr("Avg. Price"),     _priceColumn,    Qt::AlignRight,  true,  "avgprice"  );
  list()->addColumn(tr("Wt. Avg. Price"), _priceColumn,    Qt::AlignRight,  true,  "wtavgprice"  );
  list()->addColumn(tr("Total Units"),    _qtyColumn,      Qt::AlignRight,  true,  "totalunits"  );
  list()->addColumn(tr("Total Sales"),    _bigMoneyColumn, Qt::AlignRight,  true,  "totalsales"  );
  if (!omfgThis->singleCurrency())
    list()->addColumn(tr("Currency"),       _currencyColumn,  Qt::AlignLeft,  true,  "currAbbr"  );

  xtsettingsSetValue("dspSummarizedSales/units", _units->id());
}


