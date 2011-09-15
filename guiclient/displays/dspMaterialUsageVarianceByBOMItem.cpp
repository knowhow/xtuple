/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspMaterialUsageVarianceByBOMItem.h"

#include <QVariant>
#include <QMessageBox>

dspMaterialUsageVarianceByBOMItem::dspMaterialUsageVarianceByBOMItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : display(parent, "dspMaterialUsageVarianceByBOMItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Material Usage Variance by Bill of Materials Item"));
  setListLabel(tr("Material Usage Variance"));
  setReportName("MaterialUsageVarianceByBOMItem");
  setMetaSQLOptions("workOrderVariance", "material");

  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateComponentItems(int)));

  _item->setType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Post Date"),      _dateColumn,  Qt::AlignCenter, true,  "posted" );
  list()->addColumn(tr("Ordered"),        _qtyColumn,   Qt::AlignRight,  true,  "ordered"  );
  list()->addColumn(tr("Produced"),       _qtyColumn,   Qt::AlignRight,  true,  "received"  );
  list()->addColumn(tr("Proj. Req."),     _qtyColumn,   Qt::AlignRight,  true,  "projreq"  );
  list()->addColumn(tr("Proj. Qty. per"), _qtyColumn,   Qt::AlignRight,  true,  "projqtyper"  );
  list()->addColumn(tr("Act. Iss."),      _qtyColumn,   Qt::AlignRight,  true,  "actiss"  );
  list()->addColumn(tr("Act. Qty. per"),  _qtyColumn,   Qt::AlignRight,  true,  "actqtyper"  );
  list()->addColumn(tr("Qty. per Var."),  _qtyColumn,   Qt::AlignRight,  true,  "qtypervar"  );
  list()->addColumn(tr("%"),              _prcntColumn, Qt::AlignRight,  true,  "qtypervarpercent"  );
  list()->addColumn(tr("Notes"),              -1,       Qt::AlignLeft,   false, "womatlvar_notes");
  list()->addColumn(tr("Reference"), -1,       Qt::AlignLeft,   false, "womatlvar_ref");
}

void dspMaterialUsageVarianceByBOMItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspMaterialUsageVarianceByBOMItem::sPopulateComponentItems(int pItemid)
{
  if (pItemid != -1)
  {
    q.prepare( "SELECT bomitem_id,"
               "       (bomitem_seqnumber || '-' || item_number || ' ' || item_descrip1 || ' ' || item_descrip2) "
               "FROM bomitem, item "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (bomitem_parent_item_id=:item_id) ) "
               "ORDER BY bomitem_seqnumber;" );
    q.bindValue(":item_id", pItemid);
    q.exec();
    _componentItem->populate(q);
  }
  else
    _componentItem->clear();
}

bool dspMaterialUsageVarianceByBOMItem::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range.") );
    return false;
  }

  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Item"),
      tr("You must specify an Item.") );
    return false;
  }

  if(!_componentItem->isValid())
  {
    QMessageBox::warning(this, tr("Invalid BOM Item"),
      tr("You must specify a BOM Item.") );
    return false;
  }

  params.append("includeFormatted"); // print only?
  params.append("item_id", _item->id());
  params.append("bomitem_id", _componentItem->id());
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  return true;
}

