/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspMaterialUsageVarianceByWorkOrder.h"

#include <QVariant>
#include <QMessageBox>

#include "inputManager.h"

dspMaterialUsageVarianceByWorkOrder::dspMaterialUsageVarianceByWorkOrder(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspMaterialUsageVarianceByWorkOrder", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Material Usage Variance by Work Order"));
  setListLabel(tr("Material Usage Variance"));
  setReportName("MaterialUsageVarianceByWorkOrder");
  setMetaSQLOptions("workOrderVariance", "material");

  _wo->setType(cWoClosed);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  list()->addColumn(tr("Post Date"),          _dateColumn,  Qt::AlignCenter, true,  "posted" );
  list()->addColumn(tr("Component Item"),     -1,           Qt::AlignLeft,   true,  "componentitemnumber"   );
  list()->addColumn(tr("Description"),        -1,           Qt::AlignLeft,   true,  "componentdescrip");
  list()->addColumn(tr("Ordered"),            _qtyColumn,   Qt::AlignRight,  true,  "ordered"  );
  list()->addColumn(tr("Produced"),           _qtyColumn,   Qt::AlignRight,  true,  "received"  );
  list()->addColumn(tr("Proj. Req."),         _qtyColumn,   Qt::AlignRight,  true,  "projreq"  );
  list()->addColumn(tr("Proj. Qty. per"),     _qtyColumn,   Qt::AlignRight,  true,  "projqtyper"  );
  list()->addColumn(tr("Act. Iss."),          _qtyColumn,   Qt::AlignRight,  true,  "actiss"  );
  list()->addColumn(tr("Act. Qty. per"),      _qtyColumn,   Qt::AlignRight,  true,  "actqtyper"  );
  list()->addColumn(tr("Qty. per Var."),      _qtyColumn,   Qt::AlignRight,  true,  "qtypervar"  );
  list()->addColumn(tr("%"),                  _prcntColumn, Qt::AlignRight,  true,  "qtypervarpercent");
  list()->addColumn(tr("Notes"),              -1,           Qt::AlignLeft,   false, "womatlvar_notes");
  list()->addColumn(tr("Reference"), -1,           Qt::AlignLeft,   false, "womatlvar_ref");
}

void dspMaterialUsageVarianceByWorkOrder::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspMaterialUsageVarianceByWorkOrder::setParams(ParameterList &params)
{
  if(!_wo->isValid())
  {
    QMessageBox::warning(this, tr("Invalid W/O"),
      tr("You must specify a Work Order.") );
    return false;
  }

  params.append("includeFormatted"); // report only?
  params.append("wo_id", _wo->id());

  return true;
}


