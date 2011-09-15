/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCapacityUOMsByClassCode.h"

#include "guiclient.h"
#include "xtreewidget.h"

dspCapacityUOMsByClassCode::dspCapacityUOMsByClassCode(QWidget* parent, const char*, Qt::WFlags fl)
    : dspCapacityUOMsByParameter(parent, "dspCapacityUOMsByClassCode", fl)
{
  setWindowTitle(tr("Capacity UOMs by Class Code"));
  setReportName("CapacityUOMsByClassCode");

  _parameter->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Class Code"),   _itemColumn, Qt::AlignCenter,true, "classcode_code");
  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("Inv. UOM"),      _uomColumn, Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Cap. UOM"),      _uomColumn, Qt::AlignCenter,true, "capuom");
  list()->addColumn(tr("Cap./Inv. Rat."),_qtyColumn, Qt::AlignRight, true, "capinvrat");
  list()->addColumn(tr("Alt. UOM"),      _uomColumn, Qt::AlignCenter,true, "altcapuom");
  list()->addColumn(tr("Alt/Inv Ratio"), _qtyColumn, Qt::AlignRight, true, "altcapinvrat");
}

bool dspCapacityUOMsByClassCode::setParams(ParameterList &params)
{
  params.append("getclasscode");

  return dspCapacityUOMsByParameter::setParams(params);
}

