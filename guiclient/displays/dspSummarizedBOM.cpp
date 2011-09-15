/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBOM.h"

#include <QVariant>

dspSummarizedBOM::dspSummarizedBOM(QWidget* parent, const char*, Qt::WFlags fl)
  : dspBOMBase(parent, "dspSummarizedBOM", fl)
{
  setWindowTitle(tr("Summarized Bill of Materials"));
  setReportName("SummarizedBOM");

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  list()->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  list()->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  list()->addColumn(tr("Ext. Qty. Req."),_qtyColumn, Qt::AlignRight, true, "bomdata_qtyreq");
  list()->setIndentation(10);
}

bool dspSummarizedBOM::setParams(ParameterList &params)
{
  params.append("bySummarized");
  return dspBOMBase::setParams(params);
}

