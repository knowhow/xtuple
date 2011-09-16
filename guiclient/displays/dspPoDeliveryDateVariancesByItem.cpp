/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoDeliveryDateVariancesByItem.h"

#include <QMessageBox>

dspPoDeliveryDateVariancesByItem::dspPoDeliveryDateVariancesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoDeliveryDateVariancesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Order Delivery Date Variances by Item"));
  setListLabel(tr("Purchase Item Receipts"));
  setReportName("DeliveryDateVariancesByItem");
  setMetaSQLOptions("poDeliveryDateVariances", "detail");

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);
  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
  
  list()->addColumn(tr("P/O #"),         _orderColumn, Qt::AlignRight, true, "recv_order_number");
  list()->addColumn(tr("Vendor"),        _orderColumn, Qt::AlignLeft,  true, "vend_name");
  list()->addColumn(tr("Vend. Item #"),   _itemColumn, Qt::AlignLeft,  true, "itemnumber");
  list()->addColumn(tr("Vendor Description"),      -1, Qt::AlignLeft,  true, "itemdescrip");
  list()->addColumn(tr("Qty."),            _qtyColumn, Qt::AlignRight, true, "recv_qty");
  list()->addColumn(tr("Req. Due"),      _dateColumn,  Qt::AlignCenter,true, "release_duedate"  );
  list()->addColumn(tr("Req. Leadtime"), _dateColumn,  Qt::AlignRight, true, "req_leadtime"  );
  list()->addColumn(tr("Agrd. Due"),     _dateColumn,  Qt::AlignCenter,true, "argd_duedate"  );
  list()->addColumn(tr("Agrd. Leadtime"),_dateColumn,  Qt::AlignRight, true, "agrd_leadtime"  );
  list()->addColumn(tr("Recv. Date"),    _dateColumn,  Qt::AlignCenter,true, "receivedate"  );
  list()->addColumn(tr("Real Leadtime"), _dateColumn,  Qt::AlignRight, true, "real_leadtime"  );
  list()->addColumn(tr("Req. Diff"),     _dateColumn,  Qt::AlignRight, true, "req_diff"  );
  list()->addColumn(tr("Agrd. Diff"),    _dateColumn,  Qt::AlignRight, true, "argd_diff"  );
}

void dspPoDeliveryDateVariancesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoDeliveryDateVariancesByItem::setParams(ParameterList &pParams)
{
  pParams.append("byItem");
  pParams.append("item_id", _item->id());

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  return true;
}

