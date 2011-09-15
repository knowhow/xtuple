/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoReturnsByVendor.h"

#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"

dspPoReturnsByVendor::dspPoReturnsByVendor(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoReturnsByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Rejected Material by Vendor"));
  setListLabel(tr("Rejected Material"));
  setReportName("RejectedMaterialByVendor");
  setMetaSQLOptions("poReturns", "detail");
  setUseAltId(true);

  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  list()->addColumn(tr("P/O #"),              _orderColumn, Qt::AlignRight,  true,  "poreject_ponumber"  );
  list()->addColumn(tr("Vendor"),             _orderColumn, Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Date"),               _dateColumn,  Qt::AlignCenter, true,  "poreject_date" );
  list()->addColumn(tr("Vend. Item #"),       _itemColumn,  Qt::AlignLeft,   true,  "poreject_vend_item_number"   );
  list()->addColumn(tr("Vendor Description"), -1,           Qt::AlignLeft,   true,  "poreject_vend_item_descrip"   );
  list()->addColumn(tr("Qty."),               _qtyColumn,   Qt::AlignRight,  true,  "poreject_qty"  );
  list()->addColumn(tr("Reject Code"),        _itemColumn,  Qt::AlignRight,  true,  "rjctcode_code"  );
}

void dspPoReturnsByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoReturnsByVendor::setParams(ParameterList &params)
{

  if(!_vendor->isValid())
  {
    QMessageBox::warning(this, tr("Vendor Required"),
      tr("You must specify a Vendor."));
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr( "Please enter a valid Start Date." ) );
    _dates->setFocus();
    return false ;
  }
  else if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr( "Please eneter a valid End Date." ) );
    _dates->setFocus();
    return false ;
  }
  else
    _dates->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
  {
    params.append("username", _agent->currentText()); // metasql only?
    params.append("agentUsername", _agent->currentText()); // report only?
  }

  params.append("vend_id", _vendor->id());
  _warehouse->appendValue(params);

  return true;
}
