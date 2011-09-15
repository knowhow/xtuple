/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistory.h"

#include <QVariant>
#include <QMessageBox>

#include "parameterwidget.h"
#include "xtreewidget.h"
#include "dspSalesHistory.h"

dspBriefSalesHistory::dspBriefSalesHistory(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspBriefSalesHistory", fl)
{
  setWindowTitle(tr("Brief Sales History"));
  setReportName("BriefSalesHistory");
  setMetaSQLOptions("briefSalesHistory", "detail");
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate(), true  );
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate(), true);
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

  list()->addColumn(tr("Cust. #"),     _itemColumn,     Qt::AlignLeft,   true,  "cust_number"   );
  list()->addColumn(tr("Name"),        -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Cust. Type"),  _orderColumn,    Qt::AlignLeft,   true,  "custtype_code"   );
  list()->addColumn(tr("Doc. #"),      _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  list()->addColumn(tr("Invoice #"),   _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  list()->addColumn(tr("Ord. Date"),   _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  list()->addColumn(tr("Invc. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  list()->addColumn(tr("Total"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extended"  );
}

void dspBriefSalesHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->id() > -1)
    menuItem = pMenu->addAction(tr("View Sales Detail..."), this, SLOT(sViewHistory()));

}

void dspBriefSalesHistory::sViewHistory()
{
  ParameterList params;
  params.append("cohead_id", list()->id());
  params.append("run");

  dspSalesHistory *newdlg = new dspSalesHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

