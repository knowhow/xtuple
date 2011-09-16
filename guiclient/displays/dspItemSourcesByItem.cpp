/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSourcesByItem.h"

#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QMessageBox>

#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByItem.h"
#include "guiclient.h"

dspItemSourcesByItem::dspItemSourcesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspItemSourcesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Sources by Item"));
  setListLabel(tr("Item Sources"));
  setReportName("ItemSourcesByItem");
  setMetaSQLOptions("itemSources", "detail");
  setUseAltId(true);

  _item->setType(ItemLineEdit::cGeneralPurchased);

  list()->addColumn(tr("Vendor"),      -1,          Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter, true,  "itemsrc_vend_uom" );
  list()->addColumn(tr("UOM Ratio"),   _qtyColumn,  Qt::AlignRight,  true,  "itemsrc_invvendoruomratio"  );

  _item->setFocus();
}

void dspItemSourcesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspItemSourcesByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction("Edit...",           this, SLOT(sEdit()));
  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));
  menuThis->addAction("View P/Os...",      this, SLOT(sViewPOs()));
}

void dspItemSourcesByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSourcesByItem::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", list()->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSourcesByItem::sViewPOs()
{
  ParameterList params;
  params.append("itemsrc_id", list()->id());
  params.append("run");
  
  dspPoItemsByItem *newdlg = new dspPoItemsByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspItemSourcesByItem::setParams(ParameterList & params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter Item"),
                        tr("Please enter a valid Item.") );
    _item->setFocus();
    return false;
  }
  params.append("byItem"); // metasql only?
  params.append("item_id", _item->id());

  return true;
}

