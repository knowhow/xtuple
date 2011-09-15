/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostHistory.h"

#include <QMessageBox>

dspItemCostHistory::dspItemCostHistory(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspItemCostHistory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Costs History"));
  setListLabel(tr("Costing History"));
  setReportName("ItemCostHistory");
  setMetaSQLOptions("itemCost", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Element"),              -1, Qt::AlignLeft,  true, "costelem_type");
  list()->addColumn(tr("Lower"),       _costColumn, Qt::AlignCenter,true, "lowlevel");
  list()->addColumn(tr("Type"),        _costColumn, Qt::AlignLeft,  true, "type");
  list()->addColumn(tr("Time"),    _timeDateColumn, Qt::AlignCenter,true, "costhist_date");
  list()->addColumn(tr("User"),         _qtyColumn, Qt::AlignCenter,true, "username");
  list()->addColumn(tr("Old"),         _costColumn, Qt::AlignRight, true, "costhist_oldcost");
  list()->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "oldcurr");
  list()->addColumn(tr("New"),         _costColumn, Qt::AlignRight, true, "costhist_newcost");
  list()->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "newcurr");

  if (omfgThis->singleCurrency())
  {
    list()->hideColumn("oldcurr");
    list()->hideColumn("newcurr");
  }
}

void dspItemCostHistory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspItemCostHistory::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning( this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  params.append("byHistory");

  params.append("actual", tr("Actual"));
  params.append("standard", tr("Standard"));
  params.append("delete", tr("Delete"));
  params.append("new", tr("New"));

  params.append("item_id", _item->id());
 
  return true;
}
