/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCapacityUOMsByProductCategory.h"

#include "xtreewidget.h"

dspCapacityUOMsByProductCategory::dspCapacityUOMsByProductCategory(QWidget* parent, const char*, Qt::WFlags fl)
    : dspCapacityUOMsByParameter(parent, "dspCapacityUOMsByProductCategory", fl)
{
  setWindowTitle(tr("Capacity UOMs by Product Category"));
  setReportName("CapacityUOMsByProductCategory");

  _parameter->setType(ParameterGroup::ProductCategory);

  list()->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),    -1,          Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("Inv. UOM"),       _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Cap. UOM"),       _uomColumn,  Qt::AlignCenter,true, "capuom");
  list()->addColumn(tr("Cap./Inv. Rat."), _qtyColumn,  Qt::AlignRight, true, "capinvrat");
  list()->addColumn(tr("Alt. UOM"),       _uomColumn,  Qt::AlignCenter,true, "altcapuom");
  list()->addColumn(tr("Alt/Inv Ratio"),  _qtyColumn,  Qt::AlignRight, true, "altcapinvrat");
}

