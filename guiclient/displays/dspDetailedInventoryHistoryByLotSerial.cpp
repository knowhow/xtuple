/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspDetailedInventoryHistoryByLotSerial.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"

dspDetailedInventoryHistoryByLotSerial::dspDetailedInventoryHistoryByLotSerial(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspDetailedInventoryHistoryByLotSerial", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Detailed Inventory History by Lot/Serial #"));
  setListLabel(tr("Inventory History"));
  setReportName("DetailedInventoryHistoryByLotSerial");
  setMetaSQLOptions("detailedInventoryHistory", "detail");
  setUseAltId(true);

  connect(_selected, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_pattern, SIGNAL(clicked()), this, SLOT(sSelect()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);

  list()->addColumn(tr("Site"),         _whsColumn,          Qt::AlignCenter, true,  "lshist_warehous_code" );
  list()->addColumn(tr("Date"),         (_dateColumn + 30),  Qt::AlignRight,  true,  "lshist_transdate"  );
  list()->addColumn(tr("Type"),         _transColumn,        Qt::AlignCenter, true,  "lshist_transtype" );
  list()->addColumn(tr("Order #"),      _orderColumn,        Qt::AlignLeft,   true,  "lshist_ordernumber"   );
  list()->addColumn(tr("Item Number"),  _itemColumn,         Qt::AlignLeft,   true,  "lshist_item_number"   );
  list()->addColumn(tr("Location"),     _dateColumn,         Qt::AlignLeft,   true,  "lshist_locationname"   );
  list()->addColumn(tr("Lot/Serial #"), -1,                  Qt::AlignLeft,   true,  "lshist_lotserial"   );
  list()->addColumn(tr("UOM"),          _uomColumn,          Qt::AlignCenter, true,  "lshist_invuom" );
  list()->addColumn(tr("Trans-Qty"),    _qtyColumn,          Qt::AlignRight,  true,  "lshist_transqty"  );
  list()->addColumn(tr("Qty. Before"),  _qtyColumn,          Qt::AlignRight,  true,  "lshist_qty_before"  );
  list()->addColumn(tr("Qty. After"),   _qtyColumn,          Qt::AlignRight,  true,  "lshist_qty_after"  );
}

void dspDetailedInventoryHistoryByLotSerial::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLotSerial::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(2));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", list()->id());

  if (transType == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "CC")
  {
    countTag newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspDetailedInventoryHistoryByLotSerial::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(2));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->addAction(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()));
}

void dspDetailedInventoryHistoryByLotSerial::sFillList()
{
  display::sFillList();
  list()->expandAll();
}

void dspDetailedInventoryHistoryByLotSerial::sSelect()
{
  if (_selected->isChecked())
  {
    _lotSerial->setEnabled(true);
    _lotSerialPattern->setEnabled(false);
  }
  else if (_pattern->isChecked())
  {
    _lotSerial->setEnabled(false);
    _lotSerialPattern->setEnabled(true);
  }
}

bool dspDetailedInventoryHistoryByLotSerial::setParams(ParameterList &params)
{
  QString trace;

  if (_dateGroup->isChecked())
  {
    if (!_dates->startDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter Start Date"),
                             tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return false;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter End Date"),
                             tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return false;
    }
  }

  if ( ((_selected->isChecked() && _lotSerial->number().trimmed().length() == 0)
     || (_pattern->isChecked() && _lotSerialPattern->text().trimmed().length() == 0))
     && (!_item->isValid()) )
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial or Item criteria to print Inventory "
                 "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return false;
  }

  if (_traceGroup->isChecked())
  {
    if (_forward->isChecked())
      trace="F";
    else
      trace="B";
  }
  else
    trace="N";

  if (_dateGroup->isChecked())
    _dates->appendValue(params);
  if (_item->isValid())
    params.append("itemid", _item->id());
  if (_warehouse->isSelected())
    params.append("warehouseid", _warehouse->id());
  params.append("transType", _transType->id());
  params.append("trace", trace);
  if (_selected->isChecked())
  {
    if (_lotSerial->number().trimmed().length() > 0)
      params.append("lotSerial", _lotSerial->number().trimmed());
  }
  else
  {
    params.append("pattern", "t");
    params.append("lotSerial", _lotSerialPattern->text().trimmed());
  }

  return true;
}

