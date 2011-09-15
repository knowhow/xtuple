/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPendingAvailability.h"

#include <QVariant>
#include <QMessageBox>

dspPendingAvailability::dspPendingAvailability(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPendingAvailability", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Pending W/O Material Availability"));
  setListLabel(tr("Material Requirements"));
  setReportName("PendingWOMaterialAvailability");
  setMetaSQLOptions("pendingAvailability", "detail");
  setUseAltId(true);

  connect(_item, SIGNAL(privateIdChanged(int)), _warehouse, SLOT(findItemsites(int)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cTooling);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured);

  _effective->setNullString(tr("Now"));
  _effective->setNullDate(QDate::currentDate());
  _effective->setAllowNullDate(true);
  _effective->setNull();

  _buildDate->setNullString(tr("Latest"));
  _buildDate->setNullDate(omfgThis->endOfTime());
  _buildDate->setAllowNullDate(true);
  _buildDate->setNull();

  _qtyToBuild->setValidator(omfgThis->qtyVal());
  _qtyToBuild->setText("1.0");

  list()->addColumn(tr("#"),            _seqColumn,  Qt::AlignCenter, true,  "bomitem_seqnumber" );
  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "item_descrip"   );
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Pend. Alloc."), _qtyColumn,  Qt::AlignRight,  true,  "pendalloc"  );
  list()->addColumn(tr("Total Alloc."), _qtyColumn,  Qt::AlignRight,  true,  "totalalloc"  );
  list()->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "qoh"  );
  list()->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight,  true,  "totalavail"  );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

void dspPendingAvailability::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPendingAvailability::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  if(!_warehouse->isValid())
  {
    QMessageBox::warning(this, tr("Site Required"),
      tr("You must specify a Site."));
    return false;
  }

  params.append("buildQty", _qtyToBuild->toDouble());
  params.append("buildDate", _buildDate->date());

  params.append("warehous_id", _warehouse->id());
  params.append("item_id", _item->id());

  if (!(_effective->isNull()))
    params.append("effective",  _effective->date());

  if (_showShortages->isChecked())
    params.append("showShortages");

  params.append("showIndented");

  return true;
}
