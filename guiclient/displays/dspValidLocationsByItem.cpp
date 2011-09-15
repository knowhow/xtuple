/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspValidLocationsByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

dspValidLocationsByItem::dspValidLocationsByItem(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspValidLocationsByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Valid Locations by Item"));
  setListLabel(tr("Valid Locations"));
  setReportName("ValidLocationsByItem");
  setMetaSQLOptions("validLocationsByItem", "detail");

  list()->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Location"),    _itemColumn,  Qt::AlignLeft,   true,  "locationname"   );
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "locationdescrip"   );
  list()->addColumn(tr("Restricted"),  _orderColumn, Qt::AlignCenter, true,  "location_restrict"  );
  list()->addColumn(tr("Netable"),     _orderColumn, Qt::AlignCenter, true,  "location_netable" );

  _item->setFocus();
}

void dspValidLocationsByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspValidLocationsByItem::setParams(ParameterList & params)
{
  if (! _item->isValid())
  { 
    QMessageBox::warning(this, tr("Invalid Item"),
                         tr("Please select a valid Item."));
    _item->setFocus();
    return false;
  }
  params.append("item_id", _item->id());

  _warehouse->appendValue(params);

  return true;
}
