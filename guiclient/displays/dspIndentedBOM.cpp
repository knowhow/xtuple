/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspIndentedBOM.h"

#include <QVariant>

dspIndentedBOM::dspIndentedBOM(QWidget* parent, const char*, Qt::WFlags fl)
  : dspBOMBase(parent, "dspIndentedBOM", fl)
{
  setWindowTitle(tr("Indented Bill of Materials"));
  setReportName("IndentedBOM");

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Seq #"),        80,           Qt::AlignCenter,true, "bomdata_bomwork_seqnumber");
  list()->addColumn(tr("Item Number"),  _itemColumn,  Qt::AlignLeft,  true, "bomdata_item_number");
  list()->addColumn(tr("Description"),  -1,           Qt::AlignLeft,  true, "bomdata_itemdescription");
  list()->addColumn(tr("UOM"),          _uomColumn,   Qt::AlignCenter,true, "bomdata_uom_name");
  list()->addColumn(tr("Qty. Req."),    _qtyColumn,   Qt::AlignRight, true, "bomdata_qtyreq");
  list()->addColumn(tr("Scrap %"),      _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  list()->addColumn(tr("Effective"),    _dateColumn,  Qt::AlignCenter,true, "bomdata_effective");
  list()->addColumn(tr("Expires"),      _dateColumn,  Qt::AlignCenter,true, "bomdata_expires");
  list()->addColumn(tr("Notes"),        _itemColumn,  Qt::AlignCenter,false, "bomdata_notes");
  list()->addColumn(tr("Reference"),    _itemColumn,  Qt::AlignCenter,false, "bomdata_ref");
  list()->setIndentation(10);
  list()->setPopulateLinear();
}

bool dspIndentedBOM::setParams(ParameterList &params)
{
  params.append("byIndented");
  return dspBOMBase::setParams(params);
}

void dspIndentedBOM::sFillList()
{
  dspBOMBase::sFillList();
  list()->expandAll();
}

