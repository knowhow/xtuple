/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoHistory.h"

#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include "copyPurchaseOrder.h"

dspPoHistory::dspPoHistory(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoHistory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Order History"));
  setListLabel(tr("P/O Items"));
  setReportName("POHistory");
  setMetaSQLOptions("poHistory", "detail");
  setUseAltId(true);

  list()->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter, true, "poitem_linenumber");
  list()->addColumn(tr("Item/Doc. #"),  _itemColumn, Qt::AlignLeft,   true, "itemnumber");
  list()->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true, "itemdescription");
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true, "uomname");
  list()->addColumn(tr("Due/Recvd."),   _dateColumn, Qt::AlignCenter, true, "poitem_duedate");
  list()->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft,   true, "poitem_vend_item_number");
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true, "poitem_vend_uom");
  list()->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_ordered");
  list()->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_received");
  list()->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_returned");
}

void dspPoHistory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspPoHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  pMenu->addAction(tr("Copy P/O..."), this, SLOT(sCopy()));
}

void dspPoHistory::sCopy()
{
  ParameterList params;
  params.append("pohead_id", _po->id());

  copyPurchaseOrder newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

bool dspPoHistory::setParams(ParameterList &params)
{
  if(!_po->isValid())
  {
    QMessageBox::warning(this, tr("P/O Required"),
      tr("You must specify a P/O Number."));
    return false;
  }

  params.append("nonInventory", tr("Non-Inventory"));
  params.append("na", tr("N/A"));

  params.append("pohead_id", _po->id());

  return true;
}
