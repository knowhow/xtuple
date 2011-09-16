/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoDeliveryDateVariancesByVendor.h"

#include <QVariant>
#include <QMessageBox>

dspPoDeliveryDateVariancesByVendor::dspPoDeliveryDateVariancesByVendor(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoDeliveryDateVariancesByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Order Delivery Date Variances by Vendor"));
  setListLabel(tr("Purchase Item Receipts"));
  setReportName("DeliveryDateVariancesByVendor");
  setMetaSQLOptions("poDeliveryDateVariances", "detail");

  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
  
  list()->addColumn(tr("P/O #"),              _orderColumn, Qt::AlignRight,  true,  "porecv_ponumber"  );
  list()->addColumn(tr("Vendor"),             _orderColumn, Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Date"),               _dateColumn,  Qt::AlignCenter, true,  "receivedate" );
  list()->addColumn(tr("Vend. Item #"),       _itemColumn,  Qt::AlignLeft,   true,  "venditemnumber"   );
  list()->addColumn(tr("Vendor Description"), -1,           Qt::AlignLeft,   true,  "venditemdescrip"   );
  list()->addColumn(tr("Qty."),               _qtyColumn,   Qt::AlignRight,  true,  "porecv_qty"  );
  list()->addColumn(tr("Req. Due"),           _dateColumn,  Qt::AlignCenter, true,  "release_duedate"  );
  list()->addColumn(tr("Req. Leadtime"),      _dateColumn,  Qt::AlignRight,  true,  "req_leadtime"  );
  list()->addColumn(tr("Agrd. Due"),          _dateColumn,  Qt::AlignCenter, true,  "argd_duedate"  );
  list()->addColumn(tr("Agrd. Leadtime"),     _dateColumn,  Qt::AlignRight,  true,  "agrd_leadtime"  );
  list()->addColumn(tr("Recv. Date"),         _dateColumn,  Qt::AlignCenter, true,  "receivedate"  );
  list()->addColumn(tr("Real Leadtime"),      _dateColumn,  Qt::AlignRight,  true,  "real_leadtime"  );
  list()->addColumn(tr("Req. Diff"),          _dateColumn,  Qt::AlignRight,  true,  "req_diff"  );
  list()->addColumn(tr("Agrd. Diff"),         _dateColumn,  Qt::AlignRight,  true,  "argd_diff"  );
}

void dspPoDeliveryDateVariancesByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoDeliveryDateVariancesByVendor::setParams(ParameterList &params)
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning(this, tr("Vendor Required"),
      tr("You must specify a Vendor."));
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return false;
  }
  else if (!_dates->endDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return false;
  }

  params.append("vend_id", _vendor->id());
  params.append("username", _agent->currentText()); // metasql only?
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText()); // report only?

  params.append("byVendor"); // metasql only?

  return true;
}
