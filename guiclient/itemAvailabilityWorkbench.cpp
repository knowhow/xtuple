/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemAvailabilityWorkbench.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "adjustmentTrans.h"
#include "bom.h"
#include "countTag.h"
#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistoryByItem.h"
#include "dspItemCostSummary.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "expenseTrans.h"
#include "firmPlannedOrder.h"
#include "item.h"
#include "maintainItemCosts.h"
#include "materialReceiptTrans.h"
#include "mqlutil.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "reassignLotSerial.h"
#include "relocateInventory.h"
#include "scrapTrans.h"
#include "storedProcErrorLookup.h"
#include "transactionInformation.h"
#include "transferTrans.h"
#include "workOrder.h"

itemAvailabilityWorkbench::itemAvailabilityWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_availPrint,	SIGNAL(clicked()), this, SLOT(sPrintAvail()));
  connect(_availability,SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuRunning(QMenu*,QTreeWidgetItem*)));
  connect(_availability,SIGNAL(resorted()), this, SLOT(sHandleResort()));
  connect(_bomitem,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuCosted(QMenu*,QTreeWidgetItem*)));
  connect(_byDates,	SIGNAL(toggled(bool)), _endDate, SLOT(setEnabled(bool)));
  connect(_byDates,	SIGNAL(toggled(bool)), _startDate, SLOT(setEnabled(bool)));
  connect(_costedPrint,	SIGNAL(clicked()), this, SLOT(sPrintCosted()));
  connect(_costsGroup,	SIGNAL(toggled(bool)), this, SLOT(sFillListCosted()));
  connect(_costsGroupInt,SIGNAL(buttonClicked(int)), this, SLOT(sFillListCosted()));
  connect(_effective,	SIGNAL(newDate(const QDate&)), this, SLOT(sFillListWhereUsed()));
  connect(_histPrint,	SIGNAL(clicked()), this, SLOT(sPrintHistory()));
  connect(_invAvailability,SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuAvail(QMenu*,QTreeWidgetItem*)));
  connect(_invQuery,	SIGNAL(clicked()), this, SLOT(sFillListAvail()));
  connect(_invhist,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuHistory(QMenu*,QTreeWidgetItem*)));
  connect(_invhistQuery,SIGNAL(clicked()), this, SLOT(sFillListInvhist()));
  connect(_item,	SIGNAL(newId(int)), this, SLOT(sFillListCosted()));
  connect(_item,	SIGNAL(newId(int)), this, SLOT(sFillListWhereUsed()));
  connect(_item,	SIGNAL(newId(int)), this, SLOT(sClearQueries()));
  connect(_itemloc,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuLocation(QMenu*,QTreeWidgetItem*)));
  connect(_itemlocQuery,SIGNAL(clicked()), this, SLOT(sFillListItemloc()));
  connect(_locPrint,	SIGNAL(clicked()), this, SLOT(sPrintLocation()));
  connect(_runPrint,	SIGNAL(clicked()), this, SLOT(sPrintRunning()));
  connect(_showPlanned,	SIGNAL(toggled(bool)), this, SLOT(sFillListRunning()));
  connect(_showReorder,	SIGNAL(toggled(bool)), this, SLOT(sHandleShowReorder(bool)));
  connect(_warehouse,	SIGNAL(newID(int)), this, SLOT(sFillListRunning()));
  connect(_wherePrint,	SIGNAL(clicked()), this, SLOT(sPrintWhereUsed()));
  connect(_whereused,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuWhereUsed(QMenu*,QTreeWidgetItem*)));
  
  // Running Availability
  _availability->addColumn(tr("Order Type"),    _itemColumn, Qt::AlignLeft,  true, "ordertype");
  _availability->addColumn(tr("Order #"),       _itemColumn, Qt::AlignLeft,  true, "ordernumber");
  _availability->addColumn(tr("Source/Destination"),     -1, Qt::AlignLeft,  true, "item_number");
  _availability->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignLeft,  true, "duedate");
  _availability->addColumn(tr("Ordered"),        _qtyColumn, Qt::AlignRight, true, "qtyordered");
  _availability->addColumn(tr("Received"),       _qtyColumn, Qt::AlignRight, true, "qtyreceived");
  _availability->addColumn(tr("Balance"),        _qtyColumn, Qt::AlignRight, true, "balance");
  _availability->addColumn(tr("Running Avail."), _qtyColumn, Qt::AlignRight, true, "runningavail");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillListRunning()));
  
  // Inventory Availability
  _invAvailability->addColumn(tr("Site"),         -1,         Qt::AlignCenter, true, "warehous_code" );
  _invAvailability->addColumn(tr("LT"),           _whsColumn, Qt::AlignCenter, true, "itemsite_leadtime" );
  _invAvailability->addColumn(tr("QOH"),          _qtyColumn, Qt::AlignRight , true, "qoh" );
  _invAvailability->addColumn(tr("Allocated"),    _qtyColumn, Qt::AlignRight , true, "allocated" );
  _invAvailability->addColumn(tr("Unallocated"),  _qtyColumn, Qt::AlignRight , true, "unallocated" );
  _invAvailability->addColumn(tr("On Order"),     _qtyColumn, Qt::AlignRight , true, "ordered" );
  _invAvailability->addColumn(tr("Reorder Lvl."), _qtyColumn, Qt::AlignRight , true, "reorderlevel" );
  _invAvailability->addColumn(tr("OUT Level"),    _qtyColumn, Qt::AlignRight , true, "outlevel" );
  _invAvailability->addColumn(tr("Available"),    _qtyColumn, Qt::AlignRight , true, "available" );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillListAvail()));
                                                                     
  // Costed BOM
  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),       _itemColumn, Qt::AlignLeft,  true, "bomdata_bomwork_seqnumber");
  _bomitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),          -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  _bomitem->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Ext. Qty. Per"),_qtyColumn, Qt::AlignRight, true, "bomdata_qtyper");
  _bomitem->addColumn(tr("Scrap %"),    _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  _bomitem->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter,true, "bomdata_effective");
  _bomitem->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter,true, "bomdata_expires");
  _bomitem->addColumn(tr("Unit Cost"),   _costColumn, Qt::AlignRight, true, "unitcost");
  _bomitem->addColumn(tr("Ext Cost"), _priceColumn, Qt::AlignRight, true, "extendedcost");
  _bomitem->setIndentation(10);
  
  // Invhist
  _invhist->setRootIsDecorated(TRUE);
  _invhist->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft, true, "invhist_transdate");
  _invhist->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft, false, "invhist_created");
  _invhist->addColumn(tr("Type"),        _transColumn,        Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Site"),        _whsColumn,          Qt::AlignCenter,true, "warehous_code");
  _invhist->addColumn(tr("Order #/Detail"), -1,Qt::AlignLeft,  true, "orderlocation");
  _invhist->addColumn(tr("UOM"),         _uomColumn,          Qt::AlignCenter,true, "invhist_invuom");
  _invhist->addColumn(tr("Trans-Qty"),   _qtyColumn,          Qt::AlignRight, true, "transqty");
  _invhist->addColumn(tr("From Area"),   _orderColumn,        Qt::AlignLeft,  true, "locfrom");
  _invhist->addColumn(tr("QOH Before"),  _qtyColumn,          Qt::AlignRight, true, "qohbefore");
  _invhist->addColumn(tr("To Area"),     _orderColumn,        Qt::AlignLeft,  true, "locto");
  _invhist->addColumn(tr("QOH After"),   _qtyColumn,          Qt::AlignRight, false, "qohafter");
  _invhist->addColumn(tr("Cost Method"), _qtyColumn,          Qt::AlignLeft,  false, "costmethod");
  _invhist->addColumn(tr("Value Before"),_qtyColumn,          Qt::AlignRight, false, "invhist_value_before");
  _invhist->addColumn(tr("Value After"), _qtyColumn,          Qt::AlignRight, false, "invhist_value_after");
  _invhist->addColumn(tr("User"),        _orderColumn,        Qt::AlignCenter,true, "invhist_user");

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);
  
  // Itemloc
  _itemloc->addColumn(tr("Site"),          _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _itemloc->addColumn(tr("Location"),      200,          Qt::AlignLeft,   true,  "locationname"   );
  _itemloc->addColumn(tr("Netable"),       _orderColumn, Qt::AlignCenter, true,  "netable" );
  _itemloc->addColumn(tr("Lot/Serial #"),  -1,           Qt::AlignLeft,   true,  "lotserial"   );
  _itemloc->addColumn(tr("Expiration"),    _dateColumn,  Qt::AlignCenter, true,  "itemloc_expiration" );
  _itemloc->addColumn(tr("Warranty"),      _dateColumn,  Qt::AlignCenter, true,  "itemloc_warrpurc" );
  _itemloc->addColumn(tr("Qty."),          _qtyColumn,   Qt::AlignRight,  true,  "qoh"  );
  
  // Where Used
  _effective->setNullString(tr("Now"));
  _effective->setNullDate(omfgThis->dbDate());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();
  
  _whereused->addColumn(tr("Seq #"),       40,           Qt::AlignCenter, true,  "bomitem_seqnumber" );
  _whereused->addColumn(tr("Parent Item"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _whereused->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _whereused->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  _whereused->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight,  true,  "qtyper"  );
  _whereused->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight,  true,  "scrap"  );
  _whereused->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter, true,  "bomitem_effective" );
  _whereused->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter, true,  "bomitem_expires" );
  
//  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillListWhereUsed(int, bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillListWhereUsed()));
  
  // General
  _item->setFocus();

  if(!_privileges->check("ViewBOMs"))
    _tab->removeTab(5);
  if(!_privileges->check("ViewQOH"))
    _tab->removeTab(4);
  if(!_privileges->check("ViewInventoryHistory"))
    _tab->removeTab(3);
  if(!_privileges->check("ViewBOMs"))
    _tab->removeTab(2);
  if(!_privileges->check("ViewInventoryAvailability"))
  {
    _tab->removeTab(1);
    _tab->removeTab(0);
  }
  if(!_privileges->check("ViewCosts"))
  {
    _costsGroup->setEnabled(false);
    _costsGroup->setChecked(false);
  }
  else
    _costsGroup->setChecked(true);
    
  //If not Manufacturing, hide show planned option
  if (_metrics->value("Application") != "Standard")
    _showPlanned->hide();
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  
  //If not Serial Control, hide lot control
  if (!_metrics->boolean("LotSerialControl"))
    _tab->removeTab(_tab->indexOf(lotserial));
  
  if (_preferences->boolean("XCheckBox/forgetful"))
    _ignoreReorderAtZero->setChecked(true);

  _ignoreReorderAtZero->setEnabled(_showReorder->isChecked());
  
  _qoh->setPrecision(omfgThis->qtyVal());
  _orderMultiple->setPrecision(omfgThis->qtyVal());
  _reorderLevel->setPrecision(omfgThis->qtyVal());
  _orderToQty->setPrecision(omfgThis->qtyVal());

}

itemAvailabilityWorkbench::~itemAvailabilityWorkbench()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemAvailabilityWorkbench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemAvailabilityWorkbench::set( const ParameterList & pParams )
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  return NoError;
}

void itemAvailabilityWorkbench::setParams(ParameterList & params)
{
  params.append("item_id",	_item->id());
  params.append("warehous_id",	_warehouse->id());

  params.append("firmPo",	tr("Planned P/O (firmed)"));
  params.append("plannedPo",	tr("Planned P/O"));
  params.append("firmWo",	tr("Planned W/O (firmed)"));
  params.append("plannedWo",	tr("Planned W/O"));
  params.append("firmTo",	tr("Planned T/O (firmed)"));
  params.append("plannedTo",	tr("Planned T/O"));
  params.append("firmWoReq",	tr("Planned W/O Req. (firmed)"));
  params.append("plannedWoReq",	tr("Planned W/O Req."));
  params.append("pr",		tr("Purchase Request"));

  if (_showPlanned->isChecked())
    params.append("showPlanned");

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_metrics->value("Application") == "Standard")
  {
    XSqlQuery xtmfg;
    xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if (xtmfg.first())
      params.append("Manufacturing");
    params.append("showMRPplan");
  }
}

void itemAvailabilityWorkbench::sFillListWhereUsed()
{
  if ((_item->isValid()) && (_effective->isValid()))
  {
    QString sql( "SELECT bomitem_parent_item_id, item_id, bomitem_seqnumber,"
                 "       item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                 "       uom_name, itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
                 "       (bomitem_scrap * 100) AS scrap,"
                 "       bomitem_effective, bomitem_expires,"
                 "       'qtyper' AS qtyper_xtnumericrole,"
                 "       'percent' AS scrap_xtnumericrole,"
                 "       CASE WHEN COALESCE(bomitem_effective, startOfTime()) <= startOfTime() THEN :always END AS bomitem_effective_qtdisplayrole,"
                 "       CASE WHEN COALESCE(bomitem_expires, endOfTime()) >= endOfTime() THEN :never END AS bomitem_expires_qtdisplayrole "
                 "FROM bomitem, item, uom "
                 "WHERE ( (bomitem_parent_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (bomitem_item_id=:item_id)" );

    if (_effective->isNull())
      sql += "AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))";
    else
      sql += " AND (:effective BETWEEN bomitem_effective AND (bomitem_expires-1))";

    sql += ") ORDER BY item_number";

    q.prepare(sql);
    q.bindValue(":item_id", _item->id());
    q.bindValue(":effective", _effective->date());
    q.bindValue(":always", tr("Always"));
    q.bindValue(":never", tr("Never"));
    q.exec();

    //if (pLocal)
    //  _whereused->populate(q, TRUE, pItemid);
    //else
      _whereused->populate(q, TRUE);
  }
  else
    _whereused->clear();
}

void itemAvailabilityWorkbench::sFillListItemloc()
{
    if (_item->isValid())
  {
    QString sql( "SELECT itemloc_id, 1 AS type, warehous_code,"
                 "       CASE WHEN (location_id IS NULL) THEN :na"
                 "            ELSE (formatLocationName(location_id) || '-' || firstLine(location_descrip))"
                 "       END AS locationname,"
                 "       CASE WHEN (location_id IS NULL) THEN :na"
                 "            WHEN (location_netable) THEN :yes"
                 "            ELSE :no"
                 "       END AS netable,"
                 "       CASE WHEN (itemsite_controlmethod NOT IN ('L', 'S')) THEN :na"
                 "            ELSE formatlotserialnumber(itemloc_ls_id)"
                 "       END AS lotserial,"
                 "       itemloc_expiration,"
                 "       CASE WHEN (NOT itemsite_perishable) THEN :na END AS itemloc_expiration_qtdisplayrole,"
                 "       itemloc_warrpurc,"
                 "       CASE WHEN (NOT itemsite_warrpurc) THEN :na END AS itemloc_warrpurc_qtdisplayrole,"
                 "       CASE WHEN ((itemsite_perishable) AND (itemloc_expiration <= CURRENT_DATE)) THEN 'red'"
                 "       END AS itemloc_expiration_qtforegroundrole,"
                 "       CASE WHEN ((itemsite_warrpurc) AND (itemloc_warrpurc <= CURRENT_DATE)) THEN 'red'"
                 "       END AS itemloc_warrpurc_qtforegroundrole,"
                 "       itemloc_qty AS qoh,"
                 "       'qty' AS qoh_xtnumericrole "
                 "FROM itemsite, warehous,"
                 "     itemloc LEFT OUTER JOIN location ON (itemloc_location_id=location_id) "
                 "WHERE ( ( (itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S')) )"
                 " AND (itemloc_itemsite_id=itemsite_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
                 " AND (itemsite_item_id=:item_id)" );

    if (_itemlocWarehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "UNION SELECT itemsite_id, 2 AS type, warehous_code,"
           "             :na AS locationname,"
           "             :na AS netable,"
           "             :na AS lotserial,"
           "             CAST(NULL AS DATE) AS itemloc_expiration,"
           "             :na AS itemloc_expiration_qtdisplayrole,"
           "             CAST(NULL AS DATE) AS itemloc_warrpurc,"
           "             :na AS itemloc_warrpurc_qtdisplayrole,"
           "             NULL AS itemloc_expiration_qtforegroundrole,"
           "             NULL AS itemloc_warrpurc_qtforegroundrole,"
           "             itemsite_qtyonhand AS qoh,"
           "             'qty' AS qoh_xtnumericrole "
           "FROM itemsite, warehous "
           "WHERE ( (NOT itemsite_loccntrl)"
           " AND (itemsite_controlmethod NOT IN ('L', 'S'))"
           " AND (itemsite_warehous_id=warehous_id)"
           " AND (itemsite_item_id=:item_id)";

    if (_itemlocWarehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "ORDER BY warehous_code, locationname, lotserial;";

    q.prepare(sql);
    q.bindValue(":yes", tr("Yes"));
    q.bindValue(":no", tr("No"));
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":undefined", tr("Undefined"));
    q.bindValue(":item_id", _item->id());
    _itemlocWarehouse->bindValue(q);
    q.exec();
    _itemloc->populate(q, true);
  }
  else
    _itemloc->clear();
}

void itemAvailabilityWorkbench::sFillListInvhist()
{
  _invhist->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  if (_item->isValid())
  {
    ParameterList params;
    _dates->appendValue(params);
    params.append("item_id", _item->id());
    params.append("transType", _transType->id());
    _invhistWarehouse->appendValue(params);
    MetaSQLQuery mql = mqlLoad("inventoryHistory", "detail");
    q = mql.toQuery(params);
    _invhist->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void itemAvailabilityWorkbench::sFillListCosted()
{
  if (! _item->isValid() || _item->itemType() == "R")
    return;

  MetaSQLQuery mql( "SELECT bomdata_bomwork_id AS id,"
                    "       CASE WHEN bomdata_bomwork_parent_id = -1 AND "
                    "                 bomdata_bomwork_id = -1 THEN"
                    "                     -1"
                    "            ELSE bomdata_item_id"
                    "       END AS altid,"
                    "       *, "
                    "<? if exists(\"useStandardCosts\") ?> "
                    "       bomdata_stdunitcost AS unitcost, "
                    "       bomdata_stdextendedcost AS extendedcost, "
                    "<? elseif exists(\"useActualCosts\") ?> "
                    "       bomdata_actunitcost AS unitcost, "
                    "       bomdata_actextendedcost AS extendedcost, "
                    "<? endif ?> "
                    "       bomdata_qtyper, "
                    "       'qtyper' AS bomdata_qtyper_xtnumericrole,"
                    "       'percent' AS bomdata_scrap_xtnumericrole,"
                    "       'cost' AS unitcost_xtnumericrole,"
                    "       'cost' AS extendedcost_xtnumericrole,"
                    "       CASE WHEN COALESCE(bomdata_effective, startOfTime()) <= startOfTime() THEN <? value(\"always\") ?> END AS bomdata_effective_qtdisplayrole,"
                    "       CASE WHEN COALESCE(bomdata_expires, endOfTime()) <= endOfTime() THEN <? value(\"never\") ?> END AS bomdata_expires_qtdisplayrole,"
                    "       CASE WHEN bomdata_expired THEN 'expired'"
                    "            WHEN bomdata_future  THEN 'future'"
                    "       END AS qtforegroundrole,"
                    "       bomdata_bomwork_level - 1 AS xtindentrole, bomdata_itemdescription "
                    "FROM indentedbom(<? value(\"item_id\") ?>,"
                    "                 <? value(\"revision_id\") ?>,0,0)");
  ParameterList params;
  if (! setParamsCosted(params))
    return;
  q = mql.toQuery(params);
  _bomitem->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(_costsGroup->isChecked())
  {
    _bomitem->showColumn(8);
    _bomitem->showColumn(9);
    q.prepare("SELECT formatCost(SUM(bomdata_actextendedcost)) AS actextendedcost,"
              "       formatCost(SUM(bomdata_stdextendedcost)) AS stdextendedcost,"
              "       formatCost(actcost(:item_id)) AS actual,"
              "       formatCost(stdcost(:item_id)) AS standard "
              "FROM indentedbom(:item_id,"
              "                 getActiveRevId('BOM',:item_id),0,0)"
              "WHERE (bomdata_bomwork_level=1) "
              "GROUP BY actual, standard;" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      XTreeWidgetItem *last = new XTreeWidgetItem(_bomitem, -1, -1);
      last->setText(0, tr("Total Cost"));
      if(_useStandardCosts->isChecked())
        last->setText(9, q.value("stdextendedcost").toString());
      else
        last->setText(9, q.value("actextendedcost").toString());

      last = new XTreeWidgetItem( _bomitem, -1, -1);
      last->setText(0, tr("Actual Cost"));
      last->setText(9, q.value("actual").toString());

      last = new XTreeWidgetItem( _bomitem, -1, -1);
      last->setText(0, tr("Standard Cost"));
      last->setText(9, q.value("standard").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _bomitem->hideColumn(8);
    _bomitem->hideColumn(9);
  }
}


void itemAvailabilityWorkbench::sFillListRunning()
{
  _availability->clear();

  if (_item->isValid())
  {
    q.prepare( "SELECT item_type, item_sold,"
               "       itemsite_id, itemsite_qtyonhand,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty ELSE 0.0 END AS ordertoqty,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_multordqty ELSE 0.0 END AS multorderqty "
               "FROM item, itemsite "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (item_id=:item_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    if (q.first())
    {
      _qoh->setDouble(q.value("itemsite_qtyonhand").toDouble());
      _reorderLevel->setDouble(q.value("reorderlevel").toDouble());
      _orderMultiple->setDouble(q.value("multorderqty").toDouble());
      _orderToQty->setDouble(q.value("ordertoqty").toDouble());

      QString itemType            = q.value("item_type").toString();
      QString sql;

      MetaSQLQuery mql = mqlLoad("runningAvailability", "detail");
      ParameterList params;
      setParams(params);
      params.append("qoh",          q.value("itemsite_qtyonhand").toDouble());

      q = mql.toQuery(params);
      _availability->populate(q, true);
      sHandleResort();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  else
  {
    _qoh->setDouble(0);
    _reorderLevel->setDouble(0);
    _orderMultiple->setDouble(0);
    _orderToQty->setDouble(0);
  }
}

void itemAvailabilityWorkbench::sHandleResort()
{
  for (int i = 0; i < _availability->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *item = _availability->topLevelItem(i);
    if (item->data(_availability->column("runningavail"), Qt::DisplayRole).toDouble() < 0)
      item->setTextColor(_availability->column("runningavail"), namedColor("error"));
    else if (item->data(_availability->column("runningavail"), Qt::DisplayRole).toDouble() < _reorderLevel->toDouble())
      item->setTextColor(_availability->column("runningavail"), namedColor("warning"));
    else
      item->setTextColor(_availability->column("runningavail"), namedColor(""));
  }
}

void itemAvailabilityWorkbench::sHandleShowReorder( bool pValue )
{
  _ignoreReorderAtZero->setEnabled(pValue);
  if (pValue && _preferences->boolean("XCheckBox/forgetful"))
    _showShortages->setChecked(TRUE);
}


void itemAvailabilityWorkbench::sFillListAvail()
{
  _invAvailability->clear();

  if ((_byDate->isChecked()) && (!_date->isValid()))
  {
    QMessageBox::critical(this, tr("Enter Valid Date"),
                          tr("<p>You have choosen to view Inventory Availability"
			     " as of a given date but have not indicated the "
			     "date.  Please enter a valid date." ) );
    _date->setFocus();
    return;
  }

  QString sql( "SELECT itemsite_id, itemtype, warehous_id, warehous_code, itemsite_leadtime,"
               "       qoh,"
               "       noNeg(qoh - allocated) AS unallocated,"
               "       allocated,"
               "       ordered,"
               "       reorderlevel,"
               "       outlevel,"
               "       (qoh - allocated + ordered) AS available,"
               "       CASE WHEN (NOT :byLeadtime) THEN 'grey' END AS itemsite_leadtime_qtforegroundrole, "
               "       CASE "
               "         WHEN ((qoh - allocated + ordered) < 0) THEN 'error' "
               "         WHEN ((qoh - allocated + ordered) <= reorderlevel) THEN 'warning' "
               "       END AS available_qtforegroundrole, "
               "       'qty' AS qoh_xtnumericrole, "
               "       'qty' AS unallocated_xtnumericrole, "
               "       'qty' AS allocated_xtnumericrole, "
               "       'qty' AS ordered_xtnumericrole, "
               "       'qty' AS reorderlevel_xtnumericrole, "
               "       'qty' AS outlevel_xtnumericrole, "
               "       'qty' AS available_xtnumericrole "
               "FROM ( SELECT itemsite_id,"
               "              CASE WHEN (item_type IN ('P', 'O')) THEN 1"
               "                   WHEN (item_type IN ('M')) THEN 2"
               "                   ELSE 0"
               "              END AS itemtype,"
               "              warehous_id, warehous_code, itemsite_leadtime,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0 END AS reorderlevel,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_ordertoqty ELSE 0 END AS outlevel,"
               "              itemsite_qtyonhand AS qoh," );

  if (_leadTime->isChecked())
    sql += " qtyAllocated(itemsite_id, itemsite_leadtime) AS allocated,"
           " qtyOrdered(itemsite_id, itemsite_leadtime) AS ordered ";

  else if (_byDays->isChecked())
    sql += " qtyAllocated(itemsite_id, :days) AS allocated,"
           " qtyOrdered(itemsite_id, :days) AS ordered ";

  else if (_byDate->isChecked())
    sql += " qtyAllocated(itemsite_id, (:date - CURRENT_DATE)) AS allocated,"
           " qtyOrdered(itemsite_id, (:date - CURRENT_DATE)) AS ordered ";

  else if (_byDates->isChecked())
    sql += " qtyAllocated(itemsite_id, :startDate, :endDate) AS allocated,"
           " qtyOrdered(itemsite_id, :startDate, :endDate) AS ordered ";


   sql += "FROM itemsite, warehous, item "
          "WHERE ( (itemsite_active)"
          " AND (itemsite_warehous_id=warehous_id)"
          " AND (itemsite_item_id=item_id)"
          " AND (item_id=:item_id)";

  if (_invWarehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") ) AS data ";

  if (_showReorder->isChecked())
  {
    sql += "WHERE ( ((qoh - allocated + ordered) <= reorderlevel) ";

    if (_ignoreReorderAtZero->isChecked())
      sql += " AND (NOT ( ((qoh - allocated + ordered) = 0) AND (reorderlevel = 0)) ) ) ";
    else
      sql += ") ";
  }
  else if (_showShortages->isChecked())
    sql += "WHERE ((qoh - allocated + ordered) < 0) ";

  sql += "ORDER BY warehous_code DESC;";

  q.prepare(sql);
  q.bindValue(":days", _days->value());
  q.bindValue(":date", _date->date());
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":byLeadtime",QVariant(_leadTime->isChecked()));
  _invWarehouse->bindValue(q);
  q.exec();
  _invAvailability->populate(q,TRUE);
}

void itemAvailabilityWorkbench::sPrintRunning()
{
  ParameterList params;
  setParams(params);
  params.append("includeFormatted");

  orReport report("RunningAvailability", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void itemAvailabilityWorkbench::sPrintAvail()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Invalid Data"),
                      tr("You must enter a valid Item Number for this report.") );
    _item->setFocus();
    return;
  }

  ParameterList params;
  params.append("item_id", _item->id());
  _invWarehouse->appendValue(params);

  if (_leadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->text().toInt());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byDates");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }


  if(_showReorder->isChecked())
    params.append("showReorder");

  if(_ignoreReorderAtZero->isChecked())
    params.append("ignoreReorderAtZero");

  if(_showShortages->isChecked())
    params.append("showShortages");

  orReport report("InventoryAvailabilityByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

bool itemAvailabilityWorkbench::setParamsCosted(ParameterList &params)
{
  if (! _item->isValid())
  {
    QMessageBox::critical(this, tr("Select an Item"),
                          tr("Please select an Item and try again."));
    _item->setFocus();
    return false;
  }

  if  (_item->itemType() != "M" && _item->itemType() != "B" &&
       _item->itemType() != "F" && _item->itemType() != "K" &&
       _item->itemType() != "P" && _item->itemType() != "O" &&
       _item->itemType() != "L" && _item->itemType() != "J" &&
       _item->itemType() != "T")
  {
    _tab->setTabEnabled(_tab->indexOf(costed),false);
    return false;
  }
  _tab->setTabEnabled(_tab->indexOf(costed),true);

  params.append("item_id", _item->id());

  if(_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if(_useActualCosts->isChecked())
    params.append("useActualCosts");

  params.append("always", tr("Always"));
  params.append("never", tr("Never"));

  XSqlQuery rq;
  rq.prepare("SELECT getActiveRevId('BOM', :item_id) AS result;");
  rq.bindValue(":item_id", _item->id());
  rq.exec();
  if (rq.first())
  {
    int result = rq.value("result").toInt();
    if (result < -1)
    {
      systemError(this, storedProcErrorLookup("getActiveRevId", result), __FILE__, __LINE__);
      return false;
    }
    params.append("revision_id", result);
  }
  else if (rq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rq.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void itemAvailabilityWorkbench::sPrintCosted()
{
  q.prepare("SELECT indentedBOM(:item_id) AS result;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("bomworkset_id", q.value("result").toInt());
    if (! setParamsCosted(params))
      return;

    orReport report("CostedIndentedBOM", params);

    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this,
                tr("<p>Could not collect the Indented BOM information to write "
                   "write the report.</p><pre>%1</pre>")
                  .arg(q.lastError().databaseText()),
                __FILE__, __LINE__);

    return;
  }
}

void itemAvailabilityWorkbench::sPrintHistory()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Invalid Data"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _invhistWarehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("transType", _transType->id());

  params.append("average", tr("Average"));
  params.append("standard", tr("Standard"));
  params.append("job", tr("Job"));
  params.append("none", tr("None"));
  params.append("unknown", tr("Unknown"));

  orReport report("InventoryHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void itemAvailabilityWorkbench::sPrintLocation()
{
  if(!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                      tr("You must enter a valid Item Number for this report.") );
    return;
  }

  ParameterList params;

  params.append("item_id", _item->id());
  _itemlocWarehouse->appendValue(params);

  orReport report("LocationLotSerialNumberDetail", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void itemAvailabilityWorkbench::sPrintWhereUsed()
{
  ParameterList params;
  params.append("item_id", _item->id());

  if(!_effective->isNull())
    params.append("effective", _effective->date());

  orReport report("SingleLevelWhereUsed", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void itemAvailabilityWorkbench::sPopulateMenuRunning( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  if (pSelected->text(0) == tr("Planned W/O (firmed)") || pSelected->text(0) == tr("Planned W/O") ||
      pSelected->text(0) == tr("Planned P/O (firmed)") || pSelected->text(0) == tr("Planned P/O") )
  {
    if (pSelected->text(0) == tr("Planned W/O (firmed)") || pSelected->text(0) == tr("Planned P/O (firmed)") )
      pMenu->addAction(tr("Soften Order..."), this, SLOT(sSoftenOrder()));
    else
      pMenu->addAction(tr("Firm Order..."), this, SLOT(sFirmOrder()));

    pMenu->addAction(tr("Release Order..."), this, SLOT(sReleaseOrder()));
    pMenu->addAction(tr("Delete Order..."), this, SLOT(sDeleteOrder()));
  }

  else if (pSelected->text(0).contains("W/O") && !(pSelected->text(0) == tr("Planned W/O Req. (firmed)") || pSelected->text(0) == tr("Planned W/O Req.")))
    pMenu->addAction(tr("View Work Order Details..."), this, SLOT(sViewWOInfo()));
}

void itemAvailabilityWorkbench::sPopulateMenuAvail( QMenu *pMenu, QTreeWidgetItem * selected )
{
  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(selected);

  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Inventory History..."), this, SLOT(sViewHistory()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
  menuItem->setEnabled(item && item->rawValue("allocated").toDouble() != 0.0);

  menuItem = pMenu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  menuItem->setEnabled(item && item->rawValue("ordered").toDouble() != 0.0);

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));

  pMenu->addSeparator();

  if (item && (item->altId() == 1))
  {
    menuItem = pMenu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));

    menuItem = pMenu->addAction(tr("Create P/O..."), this, SLOT(sCreatePO()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    pMenu->addSeparator();
  }
  else if (item && (item->altId() == 2))
  {
    menuItem = pMenu->addAction(tr("Create W/O..."), this, SLOT(sCreateWO()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

    pMenu->addSeparator();
  }

  menuItem = pMenu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
  menuItem->setEnabled(_privileges->check("IssueCountTags"));

  menuItem = pMenu->addAction(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()));
  menuItem->setEnabled(_privileges->check("EnterMiscCounts"));

  pMenu->addSeparator();

  pMenu->addAction(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()));
}

void itemAvailabilityWorkbench::sPopulateMenuCosted( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(pSelected);
  if(! item || !_privileges->check("ViewCosts"))
    return;

  if (item->id() != -1)
    pMenu->addAction(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()));

  if (item->id() != -1)
    pMenu->addAction(tr("View Item Costing..."), this, SLOT(sViewItemCosting()));
}

void itemAvailabilityWorkbench::sPopulateMenuHistory( QMenu * pMenu, QTreeWidgetItem * pItem )
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()));
  menuItem = pMenu->addAction(tr("Edit Transaction Information..."), this, SLOT(sEditTransInfo()));

  if ( (pItem->text(3).length()) &&
       ( (pItem->text(2) == "RM") || (pItem->text(2) == "IM") ) )
  {
    QString orderNumber = _invhist->currentItem()->text(4);
    int sep1            = orderNumber.indexOf('-');
    int sep2            = orderNumber.indexOf('-', (sep1 + 1));
    int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
    int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

    if ( (mainNumber) && (subNumber) )
    {
      menuItem = pMenu->addAction(tr("View Work Order Information..."), this, SLOT(sViewWOInfoHistory()));
      menuItem->setEnabled(_privileges->check("MaintainWorkOrders") ||
                           _privileges->check("ViewWorkOrders"));
    }
  }
}

void itemAvailabilityWorkbench::sPopulateMenuLocation( QMenu * pMenu, QTreeWidgetItem * pSelected )
{
  QAction *menuItem;
  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(pSelected);

  if (item && item->altId() == -1)
  {
    menuItem = pMenu->addAction(tr("Relocate..."), this, SLOT(sRelocateInventory()));
    menuItem->setEnabled(_privileges->check("RelocateInventory"));

    menuItem = pMenu->addAction(tr("Reassign Lot/Serial #..."), this, SLOT(sReassignLotSerial()));
    menuItem->setEnabled(_privileges->check("ReassignLotSerial"));
  }
}

void itemAvailabilityWorkbench::sPopulateMenuWhereUsed( QMenu *menu, QTreeWidgetItem * )
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("Edit Bill of Materials..."), this, SLOT(sEditBOM()));
  menuItem->setEnabled(_privileges->check("MaintainBOMs"));

  menuItem = menu->addAction(tr("Edit Item Master..."), this, SLOT(sEditItem()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  menuItem = menu->addAction(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
}

void itemAvailabilityWorkbench::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byRange");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _invAvailability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _invAvailability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _invAvailability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _invAvailability->id());

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=FALSE "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", _availability->id());
  q.exec();

  sFillListRunning();
}

void itemAvailabilityWorkbench::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", _availability->id());

  firmPlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillListRunning();
}

void itemAvailabilityWorkbench::sReleaseOrder()
{
  // TODO
  if (_availability->currentItem()->text(0) == tr("Planned W/O (firmed)") || _availability->currentItem()->text(0) == tr("Planned W/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _availability->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
#if 0
    if (newdlg.exec() != XDialog::Rejected)
    {
      sDeleteOrder();
      sFillListRunning();
    }
#endif
  }
  else if (_availability->currentItem()->text(0) == tr("Planned P/O (firmed)") || _availability->currentItem()->text(0) == tr("Planned P/O"))
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _availability->id());

    purchaseRequest newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillListRunning();
  }
}

void itemAvailabilityWorkbench::sDeleteOrder()
{
  q.prepare( "SELECT deletePlannedOrder(:planord_id, FALSE);" );
  q.bindValue(":planord_id", _availability->id());
  q.exec();

  sFillListRunning();
}

void itemAvailabilityWorkbench::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());

  maintainItemCosts *newdlg  = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", _bomitem->altId() );
  params.append( "run",     TRUE              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(2));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

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
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else
  {
    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void itemAvailabilityWorkbench::sEditTransInfo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invhist_id", _invhist->id());

  transactionInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemAvailabilityWorkbench::sViewWOInfo()
{
  QString orderNumber = _availability->currentItem()->text(1);
  int sep1            = orderNumber.indexOf('-');
  int mainNumber      = orderNumber.left(sep1).toInt();
  int subNumber       = orderNumber.right(orderNumber.length() - sep1 - 1).toInt();

  q.prepare( "SELECT wo_id "
             "FROM wo "
             "WHERE ( (wo_number=:wo_number)"
             " AND (wo_subnumber=:wo_subnumber) );" );
  q.bindValue(":wo_number", mainNumber);
  q.bindValue(":wo_subnumber", subNumber);
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("wo_id"));

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void itemAvailabilityWorkbench::sViewWOInfoHistory()
{
  QString orderNumber = _invhist->currentItem()->text(4);
  int sep1            = orderNumber.indexOf('-');
  int sep2            = orderNumber.indexOf('-', (sep1 + 1));
  int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
  int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

  q.prepare( "SELECT wo_id "
             "FROM wo "
             "WHERE ( (wo_number=:wo_number)"
             " AND (wo_subnumber=:wo_subnumber) );" );
  q.bindValue(":wo_number", mainNumber);
  q.bindValue(":wo_subnumber", subNumber);
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("wo_id"));

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void itemAvailabilityWorkbench::sRelocateInventory()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  relocateInventory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillListItemloc();
}

void itemAvailabilityWorkbench::sReassignLotSerial()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  reassignLotSerial newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillListItemloc();
}

void itemAvailabilityWorkbench::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _whereused->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sEditItem()
{
  item::editItem(_whereused->id());
}

void itemAvailabilityWorkbench::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", _whereused->altId());
  params.append("warehous_id", -1);
  params.append("run");

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemAvailabilityWorkbench::sClearQueries()
{
  _invAvailability->clear();
  _invhist->clear();
  _itemloc->clear();
}


