/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspSalesHistoryByItem.h"

dspSummarizedSalesByItem::dspSummarizedSalesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedSalesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Sales History by Item"));
  setListLabel(tr("Sales History"));
  setReportName("SummarizedSalesHistoryByItem");
  setMetaSQLOptions("summarizedSalesHistory", "detail");

  _customerType->setType(ParameterGroup::CustomerType);
  _productCategory->setType(ParameterGroup::ProductCategory);

  list()->addColumn(tr("Item"),        _itemColumn,      Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,               Qt::AlignLeft,   true,  "itemdescription"   );
  list()->addColumn(tr("First Sale"),  _dateColumn,      Qt::AlignCenter, true,  "firstdate" );
  list()->addColumn(tr("Last Sale"),   _dateColumn,      Qt::AlignCenter, true,  "lastdate" );
  list()->addColumn(tr("Total Units"), _qtyColumn,       Qt::AlignRight,  true,  "totalunits"  );
  list()->addColumn(tr("Total Sales"), _bigMoneyColumn,  Qt::AlignRight,  true,  "totalsales"  );
}

void dspSummarizedSalesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSummarizedSalesByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction(tr("View Sales Detail..."), this, SLOT(sViewDetail()));
}

void dspSummarizedSalesByItem::sViewDetail()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _customerType->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", list()->id());
  params.append("run");

  dspSalesHistoryByItem *newdlg = new dspSalesHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspSummarizedSalesByItem::setParams(ParameterList & params)
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

  if (_customerType->isPattern())
  {
    QString pattern = _customerType->pattern();
    if (pattern.length() == 0)
       return false;
  }

  _dates->appendValue(params);
  _productCategory->appendValue(params);
  _customerType->appendValue(params);
  _warehouse->appendValue(params);

  params.append("byItem"); // metasql only?

  return true;
}

