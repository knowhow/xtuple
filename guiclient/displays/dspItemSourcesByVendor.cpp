/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSourcesByVendor.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByVendor.h"
#include "guiclient.h"

dspItemSourcesByVendor::dspItemSourcesByVendor(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspItemSourcesByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Sources by Vendor"));
  setListLabel(tr("Item Sources"));
  setReportName("ItemSourcesByVendor");
  setMetaSQLOptions("itemSources", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  list()->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "itemsrc_vend_uom"   );
  list()->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight,  true,  "itemsrc_invvendoruomratio"  );
}

void dspItemSourcesByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspItemSourcesByVendor::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction("Edit...",           this, SLOT(sEdit()));
  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));
}

void dspItemSourcesByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSourcesByVendor::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", list()->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspItemSourcesByVendor::setParams(ParameterList &params)
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Vendor"),
                        tr("Please enter a valid Vendor.") );
    _vendor->setFocus();
    return false;
  }

  params.append("byVendor"); // metasql only?
  params.append("vend_id", _vendor->id());

  return true;
}
