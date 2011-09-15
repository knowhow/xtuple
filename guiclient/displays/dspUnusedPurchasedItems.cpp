/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUnusedPurchasedItems.h"

#include <QMessageBox>

#include "xtreewidget.h"

dspUnusedPurchasedItems::dspUnusedPurchasedItems(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Unused Purchased Items"));
  setListLabel(tr("Unused Purchased Items"));
  setReportName("UnusedPurchasedItems");
  setMetaSQLOptions("unusedPurchasedItems", "detail");

  _classCode->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"  );
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"  );
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "uom_name"  );
  list()->addColumn(tr("Total QOH"),   _qtyColumn,  Qt::AlignRight,  true,  "qoh" );
  list()->addColumn(tr("Last Cnt'd"),  _dateColumn, Qt::AlignRight,  true,  "lastcount" );
  list()->addColumn(tr("Last Used"),   _dateColumn, Qt::AlignRight,  true,  "lastused" );
}

void dspUnusedPurchasedItems::languageChange()
{
  languageChange();
  retranslateUi(this);
}

bool dspUnusedPurchasedItems::setParams(ParameterList & params)
{
  if (_classCode->isPattern() && ! _classCode->isAll())
  {
    QString pattern = _classCode->pattern();
    if (pattern.length() == 0)
    {
      QMessageBox::warning(this, tr("Enter Class Code"),
                           tr("Class Code Pattern cannot be blank."));
	  return false;
    }
  }

  _classCode->appendValue(params);

  if (!_includeUncontrolled->isChecked())
    params.append("includeUncontrolled"); // metasql only?
  if(_includeUncontrolled->isChecked())
    params.append("includeUncontrolledItems"); // report only?

  return true;
}
