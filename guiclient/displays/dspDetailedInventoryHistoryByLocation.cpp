/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspDetailedInventoryHistoryByLocation.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"

dspDetailedInventoryHistoryByLocation::dspDetailedInventoryHistoryByLocation(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspDetailedInventoryHistoryByLocation", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Detailed Inventory History by Location"));
  setListLabel(tr("Inventory History"));
  setReportName("DetailedInventoryHistoryByLocation");
  setMetaSQLOptions("detailedInventoryHistory", "detail");
  setUseAltId(true);

  connect(_location, SIGNAL(newID(int)), this, SLOT(sPopulateLocationInfo(int)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulateLocations()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  
  if (_metrics->value("Application") != "PostBooks")
    _transType->append(cTransTransfers, tr("Transfers")            );
  
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);

  list()->addColumn(tr("Date"), (_dateColumn + 30), Qt::AlignRight, true, "invhist_transdate");
  list()->addColumn(tr("Type"),       _transColumn, Qt::AlignCenter,true, "invhist_transtype");
  list()->addColumn(tr("Order #"),    _orderColumn, Qt::AlignLeft,  true, "ordernumber");
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Lot/Serial #"), -1,         Qt::AlignLeft,  true, "lotserial");
  list()->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "invhist_invuom");
  list()->addColumn(tr("Trans-Qty"),    _qtyColumn, Qt::AlignRight, true, "transqty");
  list()->addColumn(tr("Qty. Before"),  _qtyColumn, Qt::AlignRight, true, "qohbefore");
  list()->addColumn(tr("Qty. After"),   _qtyColumn, Qt::AlignRight, true, "qohafter");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
  
  sPopulateLocations();
}

void dspDetailedInventoryHistoryByLocation::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLocation::sPopulateLocations()
{
  if (_warehouse->isAll())
    _location->populate( "SELECT location_id,"
                         "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (warehous_code || '-' || formatLocationName(location_id) || '-' || location_descrip)"
                         "            ELSE (warehous_code || '-' || formatLocationName(location_id))"
                         "       END AS locationname "
                         "FROM location, warehous "
                         "WHERE (location_warehous_id=warehous_id) "
                         "ORDER BY locationname;" );
  else
  {
    XSqlQuery qq;
    qq.prepare( "SELECT location_id, "
               "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
               "            ELSE formatLocationName(location_id)"
               "       END AS locationname "
               "FROM location "
               "WHERE (location_warehous_id=:warehous_id) "
               "ORDER BY locationname;" );
    _warehouse->bindValue(qq);
    qq.exec();
    _location->populate(qq);
  }
}

void dspDetailedInventoryHistoryByLocation::sPopulateLocationInfo(int pLocationid)
{
  XSqlQuery qq;
  qq.prepare( "SELECT formatBoolYN(location_netable) AS netable,"
             "       formatBoolYN(location_restrict) AS restricted "
             "FROM location, warehous "
             "WHERE ( (location_warehous_id=warehous_id)"
             " AND (location_id=:location_id) );" );
  qq.bindValue(":location_id", pLocationid);
  qq.exec();
  if (qq.first())
  {
    _netable->setText(qq.value("netable").toString());
    _restricted->setText(qq.value("restricted").toString());
  }
}

void dspDetailedInventoryHistoryByLocation::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(1));

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

void dspDetailedInventoryHistoryByLocation::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(1));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->addAction(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()));
}


bool dspDetailedInventoryHistoryByLocation::setParams(ParameterList &params)
{
  params.append("byLocation");
 
  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }
  else if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }
  else
    _dates->appendValue(params);

  params.append("location_id", _location->id());

  params.append("transType", _transType->id());

  return true;
}
