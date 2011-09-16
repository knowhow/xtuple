/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspJobCosting.h"

#include <QMessageBox>
#include <QVariant>

dspJobCosting::dspJobCosting(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspJobCosting", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Work Order Costing"));
  setListLabel(tr("Posted Costs"));
  setReportName("JobCosting");
  setMetaSQLOptions("manufacture", "jobcosting");
  setUseAltId(true);

  list()->addColumn(tr("Type"),            _itemColumn, Qt::AlignLeft,  true, "type");
  list()->addColumn(tr("Work Center/Item"),_itemColumn, Qt::AlignLeft,  true, "code");
  list()->addColumn(tr("Description"),   -1, Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("Qty."),  _qtyColumn, Qt::AlignRight, true, "qty");
  list()->addColumn(tr("UOM"),   _uomColumn, Qt::AlignCenter,true, "uom");
  list()->addColumn(tr("Cost"),_moneyColumn, Qt::AlignRight, true, "cost");

  _wo->setFocus();
}

void dspJobCosting::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspJobCosting::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspJobCosting::setParams(ParameterList &params)
{
  if (! _wo->isValid())
  {
   QMessageBox::warning(this, tr("Select Options"),
                        tr("<p>You must select a Work Order."));
    _wo->setFocus();
    return false;
  }

  params.append("wo_id", _wo->id());
  params.append("showsu", true);
  params.append("showrt", true);
  params.append("showmatl", true);
  params.append("setup", tr("Setup"));
  params.append("runtime", tr("Run Time"));
  params.append("material", tr("Material"));
  params.append("timeuom", tr("Hours"));

  return true;
}

