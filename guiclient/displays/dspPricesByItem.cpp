/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByItem.h"

#include <QMessageBox>

#include "guiclient.h"

#define CURR_COL	5
#define COST_COL	6
#define MARGIN_COL	7

dspPricesByItem::dspPricesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPricesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Prices by Item"));
  setListLabel(tr("Prices"));
  setReportName("PricesByItem");
  setMetaSQLOptions("prices", "detail");
  setUseAltId(true);

  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _item->setType(ItemLineEdit::cSold);

  list()->addColumn(tr("Schedule"),      _itemColumn, Qt::AlignLeft,   true,  "schedulename"  );
  list()->addColumn(tr("Source"),        _itemColumn, Qt::AlignLeft,   true,  "type"  );
  list()->addColumn(tr("Customer/Customer Type"), -1, Qt::AlignLeft,   true,  "typename"  );
  list()->addColumn(tr("Qty. Break"),     _qtyColumn, Qt::AlignRight,  true,  "f_qtybreak" );
  list()->addColumn(tr("Price"),        _priceColumn, Qt::AlignRight,  true,  "price" );
  list()->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,   true,  "currConcat"  );
  list()->addColumn(tr("Cost"),          _costColumn, Qt::AlignRight,  true,  "f_cost" );
  list()->addColumn(tr("Margin"),       _prcntColumn, Qt::AlignRight,  true,  "f_margin" );

  if (omfgThis->singleCurrency())
    list()->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());

  _item->setFocus();
}

void dspPricesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspPricesByItem::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
  {
    list()->showColumn(COST_COL);
    list()->showColumn(MARGIN_COL);
  }
  else
  {
    list()->hideColumn(COST_COL);
    list()->hideColumn(MARGIN_COL);
  }

  _costsGroup->setEnabled(pShowCosts);
}

bool dspPricesByItem::setParams(ParameterList & params)
{
  double cost = 0.0;

  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  if (_showCosts->isChecked())
  {
    if (_useStandardCosts->isChecked())
    {
      q.prepare( "SELECT (stdCost(item_id) * iteminvpricerat(item_id)) AS cost "
                 "FROM item "
                 "WHERE (item_id=:item_id);");
      params.append("standardCosts"); // report only?
    }
    else if (_useActualCosts->isChecked())
    {
      q.prepare( "SELECT (actCost(item_id) * iteminvpricerat(item_id)) AS cost "
                 "FROM item "
                 "WHERE (item_id=:item_id);");
      params.append("actualCosts"); // report only?
    }

    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
      cost = q.value("cost").toDouble();
    else
      return false;
  }

  params.append("na", tr("N/A"));
  params.append("customer", tr("Customer"));
  params.append("shipTo", tr("Cust. Ship-To"));
  params.append("shipToPattern", tr("Cust. Ship-To Pattern"));
  params.append("custType", tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));
  params.append("listPrice", tr("List Price"));
  params.append("item_id", _item->id());
  params.append("cost", cost);
  if (_showCosts->isChecked())
    params.append("showCosts");
  if (_showExpired->isChecked())
    params.append("showExpired");
  if (_showFuture->isChecked())
    params.append("showFuture");
  params.append("byItem");

  return true;
}
